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

	void NoiseField::setResolutionParam(ofParameter<int> resolution){
		this->resolution.makeReferenceTo(resolution);
	}

    //--------------------------------------------------------------
    void NoiseField::update()
    {
        for(auto & octave: octaves){
            if(octave.advanceTime){
                octave.now += noiseSpeed * ofGetLastFrameTime();
            }
        }
    }

    //--------------------------------------------------------------
    void NoiseField::numScalesChanged(int& numScales)
	{
    }

    //--------------------------------------------------------------
    float NoiseField::getValue(int x, int y, int z)
    {
		double total = 0;
        double maxValue = 0;
        glm::vec3 centeredPos = glm::vec3(x - resolution/2., y - resolution/2., z - resolution/2.);
		glm::vec3 pos = centeredPos * (float)inputMultiplier;
		for(auto & octave: octaves){
			float radius = octave.radius * resolution;
			if(octave.enabled &&  radius * radius > glm::length2(centeredPos)){
				auto freqD = octave.frequency / 60.f;
				total += ofNoise(pos.x*freqD, pos.y*freqD, pos.z*freqD, octave.now*freqD) * octave.amplitude;
				maxValue += octave.amplitude;
			}
		}

		return total/(maxValue*normalizationFactor);
    }
}
