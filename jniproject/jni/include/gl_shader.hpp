/*
 * =====================================================================================
 *
 *       Filename:  gl_shader.h
 *
 *    Description:  A class for glShader compilation and linking 
 *
 *        Version:  1.0
 *        Created:  06/11/2014 10:32:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Pablo Colapinto (), gmail -> wolftype
 *   Organization:  lynda.com 
 *
 * =====================================================================================
 */

#ifndef GL_SHADER_HPP
#define GL_SHADER_HPP

#include <EGL/egl.h> // requires ndk r5 or newer
#include <GLES2/gl2.h>
#include <iostream>

#include "logger.h"

namespace lynda {

//using namespace std;

    class Shader {
        
      GLuint sID;

      public:

      GLuint id() const { return sID; }

      Shader(const char * vert, const char * frag){

        /*-----------------------------------------------------------------------------
         *  CREATE THE SHADER
         *-----------------------------------------------------------------------------*/

        //1. CREATE SHADER PROGRAM
        sID = glCreateProgram();
        GLuint vID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fID = glCreateShader(GL_FRAGMENT_SHADER);

        //2. LOAD SHADER SOURCE CODE
        glShaderSource(vID, 1, &vert, NULL); //<-- Last argument specifies length of source string
        glShaderSource(fID, 1, &frag, NULL);

        //3. COMPILE
        glCompileShader(vID);
        glCompileShader(fID);

        //4. CHECK FOR COMPILE ERRORS
        compilerCheck(vID);
        compilerCheck(fID);

        //5. ATTACH SHADERS TO PROGRAM
        glAttachShader(sID,vID);
        glAttachShader(sID,fID);

        //6. LINK PROGRAM
        glLinkProgram(sID);	

        //7. CHECK FOR LINKING ERRORS
        linkCheck(sID);
            
        //8. USE PROGRAM
        glUseProgram(sID);
      }

      void bind(){ glUseProgram(sID); }
      void unbind() { glUseProgram(0); }


      /*-----------------------------------------------------------------------------
       *  FUNCION TO CHECK FOR SHADER COMPILER ERRORS
       *-----------------------------------------------------------------------------*/
      void compilerCheck(GLuint ID){
        GLint comp;
        glGetShaderiv(ID, GL_COMPILE_STATUS, &comp);

        if(comp == GL_FALSE ){
//            cout << "Shader Compilation FAILED" << endl;
        	LOG_INFO("Shader Compilation FAILED");
            GLchar messages[256];
            glGetShaderInfoLog(ID, sizeof(messages),0,&messages[0]);
//            cout << messages;
            LOG_INFO("%s", messages);
        }
        else {
//        	cout << "shader compile ok" << endl;
        	LOG_INFO("Shader compiles okay.");
        }
      }


      /*-----------------------------------------------------------------------------
       *  FUNCION TO CHECK FOR SHADER LINK ERRORS
       *-----------------------------------------------------------------------------*/
      void linkCheck(GLuint ID){
        GLint linkStatus, validateStatus;
        glGetProgramiv(ID, GL_LINK_STATUS, &linkStatus);
            

        if(linkStatus == GL_FALSE ){
//            cout << "Shader Linking FAILED" << endl;
        	LOG_INFO("Shader Linking FAILED");
            GLchar messages[256];
            glGetProgramInfoLog(ID, sizeof(messages),0,&messages[0]);
//            cout << messages;
            LOG_INFO("%s", messages);
        }

        glValidateProgram(ID);
        glGetProgramiv(ID, GL_VALIDATE_STATUS, &validateStatus);
          
//        cout << "Link: " << linkStatus << "  Validate: " << validateStatus << endl;
        LOG_INFO("Link: %d Validate: %d", linkStatus, validateStatus);

        if(linkStatus == GL_FALSE ){
//            cout << "Shader Validation FAILED" << endl;
        	LOG_INFO("Shader Validation FAILED");
            GLchar messages[256];
            glGetProgramInfoLog(ID, sizeof(messages),0,&messages[0]);
//            cout << messages;
            LOG_INFO("%s", messages);
        }

      }



  };



} //lynda::

#endif
