//
//  ParticleRenderer.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#include "ofxGaussianMapTexture.h"

#include "ParticleRenderer.h"

namespace entropy
{
    //--------------------------------------------------------------
    ParticleRenderer::ParticleRenderer()
    : _pointSize(1.0f)
    {
        // Load the shader.
        if (!_shader.load("shaders/render")) {
            ofLogError("ParticleRenderer::_initGL", "Error loading shader");
        }

        ofxCreateGaussianMapTexture(_texture, 32, GL_TEXTURE_2D);
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
