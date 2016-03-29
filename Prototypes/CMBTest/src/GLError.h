#pragma once

#include <string>
#include "ofMain.h"

#define CheckGLError() _check_gl_error(__FILE__,__LINE__)
//#define CheckGLError()

namespace lb {

static void _check_gl_error( const char * _file, int _line )
{
    GLenum err = glGetError();

    while ( err != GL_NO_ERROR )
    {
        std::string error;

        switch ( err ) {
        case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        }

        std::cout << "GL_" << error.c_str() << " - " << err << ", " << _file << ":" << _line << std::endl;
      //  ofExit( 0 );
        err = glGetError();
    }
}

}
