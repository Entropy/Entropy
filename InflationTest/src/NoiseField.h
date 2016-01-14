//
//  NoiseField.hpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxMarchingCubes.h"

namespace entropy
{
    class NoiseField
    {
    public:
        NoiseField();

        void update();
        void apply(ofxMarchingCubes& marchingCubes);

        int getNumScales();
        void numScalesChanged(int& numScales);

        float getValue(int x, int y, int z);

        ofParameterGroup paramGroup;

    private:
        ofParameter<int> numScales;
        ofParameter<int> currScale;
        ofParameter<float> offsetMultiplier;
        ofParameter<float> inputMultiplier;
        ofParameter<float> noiseSpeed;

        float noiseSeed;
        float noiseOffset;
    };
}
