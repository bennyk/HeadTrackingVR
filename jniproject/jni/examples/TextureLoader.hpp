/*
 * =====================================================================================
 *
 *       Filename:  TextureLoader.cpp
 *
 *    Description: load a texture from file. 
 *
 *        Version:  1.0
 *        Created:  06/16/2014 11:06:36
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Pablo Colapinto (), gmail -> wolftype
 *
 * =====================================================================================
 */

//#include "glfw_app.hpp"
#include "gl_app.hpp"
#include "gl_shader.hpp"
#include "gl_data.hpp"                                //<-- bitmap loader
#include "gl_macros.hpp"


namespace textureloaderapp {

using namespace lynda;
using namespace std;


const char * vert = GLSL(100,

  attribute vec4 position;          
  attribute vec2 textureCoordinate;                   //<-- Texture Coordinate Attribute

  varying vec2 texCoord;                              //<-- To be passed to fragment shader

  uniform mat4 model;
  uniform mat4 view;                 //<-- 4x4 Transformation Matrices
  uniform mat4 projection;

  void main(void){
    texCoord = textureCoordinate;

    gl_Position = projection * view * model * position;
  }

);

const char * frag = GLSL(100,

  uniform sampler2D texture;                        //<-- The texture itself

  varying lowp vec2 texCoord;                            //<-- coordinate passed in from vertex shader

  void main(void){
	// HACK!! we have to reverse the RGB order coz GL_BGR is not supported in ES.
    gl_FragColor =  texture2D( texture, texCoord ).bgra; //<-- look up the coordinate
  }

);


struct vec2 {
  vec2(float _x=0, float _y=0) : x(_x), y(_y) {}
  float x,y;
};


struct Vertex{
  vec2 position;
  vec2 textureCoordinate;
};


struct TextureLoaderApp : MyGlApp {

  Shader * shader;

  GLuint tID;
  GLuint arrayID;
  GLuint bufferID, elementID;
  GLuint positionID;
  GLuint textureCoordinateID;
  GLuint samplerID;

  //ID of Uniforms
  GLuint modelID, viewID, projectionID;
 

  TextureLoaderApp() : MyGlApp() { }

  virtual bool initialize(){
	 MyGlApp::initialize();

     Bitmap img("/sdcard/flower.bmp");
      
    /*-----------------------------------------------------------------------------
     *  A slab is just a rectangle with texture coordinates
     *-----------------------------------------------------------------------------*/
     //                  position      texture coord
      Vertex slab[] = { 
                        {vec2(-.8,-.8), vec2(0,0)}, //bottom-left
                        {vec2(-.8, .8), vec2(0,1)}, //top-left
                        {vec2( .8, .8), vec2(1,1)}, //top-right
                        {vec2( .8,-.8), vec2(1,0)}  //bottom-right
                      };
      
      GLubyte indices[] = {0,1,2,  // first triangle (bottom left - top left - top right)
    		               0,2,3}; // second triangle (bottom left - top right - bottom right)

      /*-----------------------------------------------------------------------------
       *  Make some rgba data (can also load a file here)
       *-----------------------------------------------------------------------------*/
      int tw = img.width; 
      int th = img.height;

      /*-----------------------------------------------------------------------------
       *  Create Shader
       *-----------------------------------------------------------------------------*/
      shader = new Shader(vert,frag);

      /*-----------------------------------------------------------------------------
       *  Get Attribute Locations
       *-----------------------------------------------------------------------------*/
      positionID = glGetAttribLocation( shader->id(), "position" );
      textureCoordinateID = glGetAttribLocation( shader->id(), "textureCoordinate");
      
      // Get uniform locations
      modelID = glGetUniformLocation(shader -> id(), "model");
      viewID = glGetUniformLocation(shader -> id(), "view");
      projectionID = glGetUniformLocation(shader -> id(), "projection");

      /*-----------------------------------------------------------------------------
       *  Generate Vertex Array Object
       *-----------------------------------------------------------------------------*/
      GENVERTEXARRAYS(1,&arrayID);
      BINDVERTEXARRAY(arrayID);

      /*-----------------------------------------------------------------------------
       *  Generate Vertex Buffer Object
       *-----------------------------------------------------------------------------*/
      glGenBuffers(1, &bufferID);
      glBindBuffer( GL_ARRAY_BUFFER, bufferID);
      glBufferData( GL_ARRAY_BUFFER,  4 * sizeof(Vertex), slab, GL_STATIC_DRAW );

      glGenBuffers(1, &elementID);
      glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementID);
      glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW );

      /*-----------------------------------------------------------------------------
       *  Enable Vertex Attributes and Point to them
       *-----------------------------------------------------------------------------*/
      glEnableVertexAttribArray(positionID);
      glEnableVertexAttribArray(textureCoordinateID);
      glVertexAttribPointer( positionID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0 );  
      glVertexAttribPointer( textureCoordinateID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) sizeof(vec2) );        

      /*-----------------------------------------------------------------------------
       *  Unbind Vertex Array Object and the Vertex Array Buffer
       *-----------------------------------------------------------------------------*/
      BINDVERTEXARRAY(0);
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      /*-----------------------------------------------------------------------------
       *  Generate Texture and Bind it
       *-----------------------------------------------------------------------------*/
      glGenTextures(1, &tID);
      glBindTexture(GL_TEXTURE_2D, tID); 

      /*-----------------------------------------------------------------------------
       *  Allocate Memory on the GPU
       *-----------------------------------------------------------------------------*/
       // target | lod | internal_format | width | height | border | format | type | data
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tw, th, 0, GL_RGB, GL_UNSIGNED_BYTE, img.pixels.data());
         
      /*-----------------------------------------------------------------------------
       *  Load data onto GPU (bitmaps flip RGB order)
       *-----------------------------------------------------------------------------*/
      // target | lod | xoffset | yoffset | width | height | format | type | data
//      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_BGR,GL_UNSIGNED_BYTE,img.pixels.data() );

      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

      /*-----------------------------------------------------------------------------
       *  Unbind texture
       *-----------------------------------------------------------------------------*/
       glBindTexture(GL_TEXTURE_2D, 0);

       return true;

  }

  void onDraw(ParcelInfo channelInfo){
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram( shader->id() );          //<-- 1. Bind Shader
      glBindTexture( GL_TEXTURE_2D, tID );   //<-- 2. Bind Texture

      BINDVERTEXARRAY(arrayID);            //<-- 3. Bind VAO

      glm::quat q = glm::angleAxis(_lookAtAngles[0], glm::vec3(0,1,0))
      				* glm::angleAxis(_lookAtAngles[2], glm::vec3(1,0,0));
      glm::vec3 forwardDir = q * glm::vec3(0,0,-1);

      glm::quat q1 = glm::angleAxis(_lookAtAngles[1], glm::vec3(0,0,1));

      // adjust the horizontal distance for IPD too.
      glm::vec3 eyePos = glm::vec3(channelInfo.getHalfIPDOffsetRatio() ,0,1);
      glm::mat4 view = glm::lookAt( eyePos, eyePos+forwardDir, q1 * glm::vec3(0,1,0) );

      glm::mat4 proj = glm::perspective( 3.14f / 3.f, channelInfo.aspectRatio(), 0.1f,-10.f);

      glUniformMatrix4fv( viewID, 1, GL_FALSE, glm::value_ptr(view) );
      glUniformMatrix4fv( projectionID, 1, GL_FALSE, glm::value_ptr(proj) );

      glm::mat4 model;
      glUniformMatrix4fv( modelID, 1, GL_FALSE, glm::value_ptr(model) );

//      glDrawArrays( GL_QUADS, 0, 4);         //<-- 4. Draw the four slab vertices
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
      BINDVERTEXARRAY(0);                  //<-- 5. Unbind the VAO

      glBindTexture( GL_TEXTURE_2D, 0);      //<-- 6. Unbind the texture
      glUseProgram( 0 );                     //<-- 7. Unbind the shader

  }

};

/*
int main(int argc, const char * argv[]){

  MyApp app;
  app.start();

  return 0;
}
 */

}
