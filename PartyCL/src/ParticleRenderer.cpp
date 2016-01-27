//
//  ParticleRenderer.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#include "ParticleRenderer.h"

namespace entropy
{
    //--------------------------------------------------------------
    inline float evalHermite(float pA, float pB, float vA, float vB, float u)
    {
        float u2=(u*u), u3=u2*u;
        float b0 = 2*u3 - 3*u2 + 1;
        float b1 = -2*u3 + 3*u2;
        float b2 = u3 - 2*u2 + u;
        float b3 = u3 - u;
        return( b0*pA + b1*pB + b2*vA + b3*vB );
    }

    //--------------------------------------------------------------
    unsigned char* createGaussianMap(int N)
    {
        float *M = new float[2*N*N];
        unsigned char *B = new unsigned char[4*N*N];
        float X,Y,Y2,Dist;
        float Incr = 2.0f/N;
        int i=0;
        int j = 0;
        Y = -1.0f;
        //float mmax = 0;
        for (int y=0; y<N; y++, Y+=Incr)
        {
            Y2=Y*Y;
            X = -1.0f;
            for (int x=0; x<N; x++, X+=Incr, i+=2, j+=4)
            {
                Dist = (float)sqrtf(X*X+Y2);
                if (Dist>1) Dist=1;
                M[i+1] = M[i] = evalHermite(1.0f,0,0,0,Dist);
                B[j+3] = B[j+2] = B[j+1] = B[j] = (unsigned char)(M[i] * 255);
            }
        }
        delete [] M;
        return(B);
    }

    //--------------------------------------------------------------
    ParticleRenderer::ParticleRenderer()
    : _pointSize(1.0f)
    {
        // Load the shader.
        if (!_shader.load("shaders/render")) {
            ofLogError("ParticleRenderer::_initGL", "Error loading shader");
        }

        // Create the sprite texture.
        float resolution = 32;
        ofTextureData textureData;
        textureData.width = resolution;
        textureData.height = resolution;
        textureData.glInternalFormat = GL_RGBA;
        textureData.textureTarget = GL_TEXTURE_2D;
        _texture.allocate(textureData);

        unsigned char *data = createGaussianMap(resolution);
        _texture.loadData(data, resolution, resolution, GL_RGBA);
    }

    //--------------------------------------------------------------
    ParticleRenderer::~ParticleRenderer()
    {
    }

    //--------------------------------------------------------------
    void ParticleRenderer::display(ofVbo& vbo, int numParticles, DisplayMode mode /* = PARTICLE_POINTS */)
    {
        ofSetColor(ofColor::white);
        glPointSize(_pointSize);

        if (mode == PARTICLE_POINTS) {
            glPointSize(_pointSize);
            vbo.disableColors();
            vbo.draw(OF_MESH_POINTS, 0, numParticles);
        }
        else {
            glPointSize(_pointSize * 2.0f);

            if (mode == PARTICLE_SPRITES) {
                vbo.disableColors();
            }
            else {
                vbo.enableColors();
            }

            ofEnableBlendMode(OF_BLENDMODE_ADD);
            ofDisableDepthTest();
            ofEnablePointSprites();

            _shader.begin();
            _shader.setUniformTexture("splatTexture", _texture, 0);
            {
                ofSetColor(ofColor::white);
                vbo.draw(OF_MESH_POINTS, 0, numParticles);
            }
            _shader.end();

            ofDisablePointSprites();
            ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        }

        glPointSize(1.0f);
    }
}
