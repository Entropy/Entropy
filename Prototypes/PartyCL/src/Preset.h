//
//  NBodyParams.h
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#pragma once

#include "ofMain.h"

namespace entropy
{
    class Preset
    {
    public:
        Preset(float timestep, float clusterScale, float velocityScale, float softening, float damping)
        : timestep(timestep)
        , clusterScale(clusterScale)
        , velocityScale(velocityScale)
        , softening(softening)
        , damping(damping)
        {}

        float timestep;
        float clusterScale;
        float velocityScale;
        float softening;
        float damping;
    };
}
