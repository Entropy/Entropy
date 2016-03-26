#pragma once

#include "ofMain.h"
#include "gli/gli.hpp"

namespace lb
{
    class DDSCubeMap
    {
    public:
        DDSCubeMap();
        ~DDSCubeMap();

        void LoadDDSTexture( const std::string& _path );
        void BindTexture( GLuint _texUnit );

    private:
        GLuint CreateTexture( const std::string& _path );

        GLuint m_texId;
    };
}
