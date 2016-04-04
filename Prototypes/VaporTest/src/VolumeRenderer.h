//
//  VolumeRenderer.h
//  Entropy
//
//  Created by Elias Zananiri on 2016-02-17.
//
//

#pragma once

#include "ofMain.h"
#include "ofxVolumetrics.h"

namespace ent
{
    class VolumeRenderer
    {
    public:
        VolumeRenderer();
        ~VolumeRenderer();

        void draw(float scale);

        bool imGui(ofDefaultVec2& windowPos, ofDefaultVec2& windowSize);

    protected:
        // Data
        void loadVolume(const string& folder);
        ofxImageSequencePlayer imageSequencePlayer;
        ofxVolumetrics volumetrics;

        // Render
        bool bDoRender;

        float xyQuality;
        float zQuality;
        float threshold;
        float density;
        int filterMode;
    };
}
