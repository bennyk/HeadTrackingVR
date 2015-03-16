/*
 * =====================================================================================
 *
 *       Filename:  Texture.cpp
 *
 *    Description:  apply textures to a rectangular slab
 *
 *        Version:  1.0
 *        Created:  06/11/2014 18:41:42
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
#include "gl_macros.hpp"

#include <vector>

namespace textureapp {

using namespace lynda;
using namespace std;

const char * vert = GLSL(100,
  attribute vec4 position;          
  attribute vec2 textureCoordinate;              //<-- Texture Coordinate Attribute

  varying vec2 texCoord;                         //<-- To be passed to fragment shader

  uniform mat4 model;
  uniform mat4 view;                 //<-- 4x4 Transformation Matrices
  uniform mat4 projection;

  void main(void){
    texCoord = textureCoordinate;

    gl_Position = projection * view * model * position;
  }

);

const char * frag = GLSL(100,

  uniform sampler2D texture;                       //<-- The texture itself

  varying lowp vec2 texCoord;                           //<-- coordinate passed in from vertex shader

  void main(void){
    gl_FragColor = texture2D( texture, texCoord ); //<-- look up the coordinate's value
  }

);


struct vec2 {
  vec2(float _x=0, float _y=0) : x(_x), y(_y) {}
  float x,y;
};

struct vec4 {
  vec4(float _x=0, float _y=0, float _z=0, float _w=0) : x(_x), y(_y), z(_z), w(_w){}
  float x,y,z,w;
};


struct Vertex{
  vec2 position;
  vec2 textureCoordinate;
};


struct TextureApp : MyGlApp {

  int th, tw;

  Shader * shader;

  GLuint tID;
  GLuint arrayID;
  GLuint bufferID, elementID;
  GLuint positionID;
  GLuint textureCoordinateID;
  GLuint samplerID;

  //ID of Uniforms
  GLuint modelID, viewID, projectionID;
 

  TextureApp() : MyGlApp() {}

  virtual bool initialize(){
	  LOG_INFO("init TextureApp");
	MyGlApp::initialize();
      
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

      GLubyte indices[] = {0,1,2, // first triangle (bottom left - top left - top right)
                           0,2,3}; // second triangle (bottom left - top right - bottom right)
      
      /*-----------------------------------------------------------------------------
       *  Make some rgba data (can also load a file here)
       *-----------------------------------------------------------------------------*/
      tw = 40; 
      th = 40;
      vector<vec4> data;

      bool checker = false;
      for (int i=0;i<tw;++i){
        float tu = (float)i/tw;
        for (int j=0;j<th;++j){
          float tv = (float)j/th;
          data.push_back( vec4(tu,0,tv,checker) );
          checker = !checker;
        }
        checker = !checker;
      }

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

     //  samplerID = glGetUniformLocation( shader->id(), "texture" );               //<-- unnecessary if only using one texture           

      /*-----------------------------------------------------------------------------
       *  Generate And Bind Vertex Array Object
       *-----------------------------------------------------------------------------*/
      GENVERTEXARRAYS(1,&arrayID);
      BINDVERTEXARRAY(arrayID);

      /*-----------------------------------------------------------------------------
       *  Generate Vertex Buffer Object
       *-----------------------------------------------------------------------------*/
      glGenBuffers(1, &bufferID);
      glBindBuffer( GL_ARRAY_BUFFER, bufferID);
      glBufferData( GL_ARRAY_BUFFER,  4 * sizeof(Vertex), slab, GL_STATIC_DRAW );

      /*-----------------------------------------------------------------------------
      *  CREATE THE ELEMENT ARRAY BUFFER OBJECT
      *-----------------------------------------------------------------------------*/
     glGenBuffers(1, &elementID);
     glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementID);
     glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLubyte), indices, GL_STATIC_DRAW );

      /*-----------------------------------------------------------------------------
       *  Enable Vertex Attributes and Point to them
       *-----------------------------------------------------------------------------*/
      glEnableVertexAttribArray(positionID);
      glEnableVertexAttribArray(textureCoordinateID);
      glVertexAttribPointer( positionID, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0 );  
      
      glVertexAttribPointer( textureCoordinateID,
                             2,
                             GL_FLOAT,
                             GL_FALSE,
                             sizeof(Vertex),
                             (void*) sizeof(vec2) );

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
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_FLOAT, NULL);
         
      /*-----------------------------------------------------------------------------
       *  Load data onto GPU
       *-----------------------------------------------------------------------------*/
      // target | lod | xoffset | yoffset | width | height | format | type | data
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_RGBA, GL_FLOAT, &(data[0]) );
      
      //Mipmaps are good -- the regenerate the texture at various scales
      // and are necessary to avoid black screen if texParameters below are not set
      glGenerateMipmap(GL_TEXTURE_2D);

      // Set these parameters to avoid a black screen
      // caused by improperly mipmapped textures
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

      /*-----------------------------------------------------------------------------
       *  Unbind texture
       *-----------------------------------------------------------------------------*/
      glBindTexture(GL_TEXTURE_2D, 0);

      return true;

  }

  virtual void onDraw(ParcelInfo channelInfo){

      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glUseProgram( shader->id() );          //<-- 1. Bind Shader
      glBindTexture( GL_TEXTURE_2D, tID );   //<-- 2. Bind Texture
    
      BINDVERTEXARRAY(arrayID);              //<-- 3. Bind VAO

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

      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
//      glDrawArrays( GL_QUADS, 0, 4);         //<-- 4. Draw the four slab vertices
      BINDVERTEXARRAY(0);                    //<-- 5. Unbind the VAO

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



/*-----------------------------------------------------------------------------
 *  tba...
 *-----------------------------------------------------------------------------*/
      /* tw = 40; */ 
      /* th = 40; */
      /* vector<vec4> data; */

      /* bool checker = false; */
      /* for (int i=0;i<tw;++i){ */
      /*   float tu = (float)i/tw; */
      /*   for (int j=0;j<th;++j){ */
      /*     float tv = (float)j/th; */
      /*     data.push_back( vec4(tu,RAND,tv,RAND) ); */
      /*     checker = !checker; */
      /*   } */
      /*   checker = !checker; */
      /* } */

	    /* glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tw, th, GL_RGBA, GL_FLOAT, &(data[0]) ); */
      /* glGenerateMipmap(GL_TEXTURE_2D); */
      /* glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //<-- or GL_LINEAR; */


