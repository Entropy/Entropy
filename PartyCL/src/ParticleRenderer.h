//
//  ParticleRenderer.hpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#pragma once

#include "ofMain.h"

namespace entropy
{
    class ParticleRenderer
    {
    public:
        enum DisplayMode
        {
            PARTICLE_POINTS = 0,
            PARTICLE_SPRITES,
            PARTICLE_SPRITES_COLOR,

            PARTICLE_NUM_MODES
        };

        ParticleRenderer();
        ~ParticleRenderer();

        void display(ofVbo& vbo, int numParticles, DisplayMode mode = PARTICLE_POINTS);

        void setPointSize(float size)
        { _pointSize = size; }

    protected: // data
        ofShader _shader;
        ofTexture _texture;
        float _pointSize;
    };
}
