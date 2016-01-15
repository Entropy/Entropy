//
//  NoiseField.cpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#include "NoiseField.h"

namespace entropy
{
    //--------------------------------------------------------------
    NoiseField::NoiseField()
    {
        numScales.addListener(this, &NoiseField::numScalesChanged);
        numScales.set("NUM SCALES", 4, 1, 4);
        currScale.set("CURR SCALE", 0, 0, numScales - 1);
        offsetMultiplier.set("OFFSET MULTIPLIER", 0.1, 0.1, 1.0);
        inputMultiplier.set("INPUT MULTIPLIER", 1.0, 0.01, 0.5);
        noiseSpeed.set("NOISE SPEED", 0, 0, 0.5);

        paramGroup.setName("NOISE FIELD");
        paramGroup.add(numScales);
        paramGroup.add(currScale);
        paramGroup.add(offsetMultiplier);
        paramGroup.add(inputMultiplier);
        paramGroup.add(noiseSpeed);

        noiseOffset = 0;
    }

    //--------------------------------------------------------------
    void NoiseField::update()
    {
        noiseOffset += noiseSpeed;
    }

    //--------------------------------------------------------------
    void NoiseField::numScalesChanged(int& numScales)
    {
        if (currScale >= numScales) {
            currScale = numScales - 1;
        }
        currScale.setMax(numScales - 1);
    }

    //--------------------------------------------------------------
    float NoiseField::getValue(int x, int y, int z)
    {
        float offset = powf(2, currScale.getMax() - currScale) * offsetMultiplier;
        int iterations = powf(2, currScale);
        float val = 0;
        for (int i = 0; i < iterations; ++i) {
            val += ofNoise(x * inputMultiplier + i * offset,
                           y * inputMultiplier + i * offset,
                           z * inputMultiplier + i * offset,
                           noiseOffset);
        }
        val /= iterations;

        return val;
    }
}