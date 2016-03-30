#pragma once

#include "ofMain.h"
#include "lb/gl/GLError.h"

namespace lb
{
    struct ViewUboData
    {
        ofMatrix4x4 invViewMatrix;
        ofVec2f     viewportDims;
        ofVec2f     rcpViewportDims;
        float       nearClip;
        float       farClip;
        float       padding[ 2 ];
    };

    struct ViewUbo
    {
    private:
        ofBufferObject  ubo;
        GLuint          binding;

    public:
        ViewUboData     data;

        ViewUbo()
            : binding( 0 )
        {}

        int Init( GLuint _binding )
        {
            binding = _binding;
            ubo.allocate( sizeof( ViewUboData ), GL_DYNAMIC_DRAW );
 
            return ubo.isAllocated() ? 0 : -1;
        }
        
        void ConfigureShader( const ofShader& _shader )
        {
            _shader.begin();
            _shader.bindUniformBlock( binding, "ViewInfoBlock" );
            _shader.end();
        }

        void Bind()
        {
            ubo.bindBase( GL_UNIFORM_BUFFER, binding );
            lb::CheckGLError();
        }

        void Update( const ofCamera& _camera )
        {
            // per frame data shared across all shaders
            data.viewportDims = ofVec2f( ofGetWindowWidth(), ofGetWindowHeight() );
            data.rcpViewportDims = 1.0f / data.viewportDims;

            data.nearClip = _camera.getNearClip();
            data.farClip = _camera.getFarClip();
            data.invViewMatrix = _camera.getModelViewMatrix().getInverse();

            ubo.updateData( sizeof( ViewUboData ), &data );
            lb::CheckGLError();
        }
    };
};