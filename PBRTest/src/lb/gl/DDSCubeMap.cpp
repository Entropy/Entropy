#include "lb/gl/DDSCubeMap.h"
#include "lb/gl/GLError.h"

lb::DDSCubeMap::DDSCubeMap()
{

}

lb::DDSCubeMap::~DDSCubeMap()
{
}

 /// Filename can be KTX or DDS files
 GLuint lb::DDSCubeMap::CreateTexture( const std::string& _path )
 {
     lb::CheckGLError(); // clear GLEW errors

     gli::texture Texture = gli::load( _path );

     if ( Texture.empty() )
     {
         return 0;
     }

     gli::gl GL( gli::gl::PROFILE_GL33 );
     gli::gl::format const Format = GL.translate( Texture.format(), Texture.swizzles() );
     ofLogNotice() << Format.Internal << endl;

     lb::CheckGLError(); // clear GLEW errors

     GLenum Target = GL.translate( Texture.target() );

     GLuint TextureName = 0;
     glGenTextures( 1, &TextureName );
     glBindTexture( Target, TextureName );
     glTexParameteri( Target, GL_TEXTURE_BASE_LEVEL, 0 );
     glTexParameteri( Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>( Texture.levels() - 1 ) );
     glTexParameteri( Target, GL_TEXTURE_SWIZZLE_R, Format.Swizzles[ 0 ] );
     glTexParameteri( Target, GL_TEXTURE_SWIZZLE_G, Format.Swizzles[ 1 ] );
     glTexParameteri( Target, GL_TEXTURE_SWIZZLE_B, Format.Swizzles[ 2 ] );
     glTexParameteri( Target, GL_TEXTURE_SWIZZLE_A, Format.Swizzles[ 3 ] );

     lb::CheckGLError(); // clear GLEW errors

     glm::tvec3<GLsizei> const Extent( Texture.extent() );
     GLsizei const FaceTotal = static_cast<GLsizei>( Texture.layers() * Texture.faces() );

     if ( Texture.target() == gli::TARGET_CUBE )
     {
         glTexStorage2D( Target, static_cast<GLint>( Texture.levels() ), Format.Internal, Extent.x, Extent.y );
     }
     else
     {
         return 0;
     }

    lb::CheckGLError(); // clear GLEW errors

    for ( std::size_t Layer = 0; Layer < Texture.layers(); ++Layer )
    {
        for ( std::size_t Face = 0; Face < Texture.faces(); ++Face )
        {
            for ( std::size_t Level = 0; Level < Texture.levels(); ++Level )
            {
                GLsizei const LayerGL = static_cast<GLsizei>( Layer );
                glm::tvec3<GLsizei> Extent( Texture.extent( Level ) );
                Target = static_cast<GLenum>( GL_TEXTURE_CUBE_MAP_POSITIVE_X + Face );

                // upload each face / mip
                glTexSubImage2D(
                    Target, static_cast<GLint>( Level ),
                    0, 0,
                    Extent.x,
                    Extent.y,
                    Format.External, Format.Type,
                    Texture.data( Layer, Face, Level ) );

                ofLogNotice() << Layer << ", " << Face << ", " << Level << ", " <<
                    Extent.x << ", " << Extent.y << ", " << Format.External << ", " << Format.Type << endl;
            }
        }
    }

    lb::CheckGLError(); // clear GLEW errors

    return TextureName;
}


void lb::DDSCubeMap::LoadDDSTexture( const std::string& _path )
{
    m_texId = CreateTexture( ofToDataPath( _path ) );
    ofLogNotice() << m_texId << endl;
}

void lb::DDSCubeMap::BindTexture( GLuint _texUnit )
{
    glActiveTexture( GL_TEXTURE0 + _texUnit );
    glBindTexture( GL_TEXTURE_CUBE_MAP, m_texId );
}

/*GLuint loadCubemap( vector<const GLchar*> faces )
{
    GLuint textureID;
    glGenTextures( 1, &textureID );
    glActiveTexture( GL_TEXTURE0 );

    int width, height;
    unsigned char* image;

    glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );
    for ( GLuint i = 0; i < faces.size(); i++ )
    {
        image = SOIL_load_image( faces[ i ], &width, &height, 0, SOIL_LOAD_RGB );
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
            GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
            );
    }
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

    return textureID;
}
*/