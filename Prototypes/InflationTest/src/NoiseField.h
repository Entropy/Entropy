//
//  NoiseField.hpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#pragma once

#include "ofMain.h"

namespace entropy
{
	struct Octave{
		Octave(size_t idx, float frequency, float amplitude, float radius)
		    :frequency("freq. hz", frequency, 0, 200)
		    ,amplitude("ampl. norm", amplitude, 0, 1)
		    ,radius("radius", radius, 0, 1)
		    ,enabled("enabled", true)
		    ,parameters("octave " + ofToString(idx), this->frequency, this->amplitude, this->radius, this->enabled){}

		ofParameter<float> frequency;
		ofParameter<float> amplitude;
		ofParameter<float> radius;
		ofParameter<bool> enabled;
		ofParameterGroup parameters;
	};

    class NoiseField
    {
    public:
        NoiseField();

        void update();

        int getNumScales();
        void numScalesChanged(int& numScales);

        float getValue(int x, int y, int z);
		void setResolutionParam(ofParameter<int> resolution);


	private:
		ofParameter<float> offsetMultiplier{"offset multiplier", 0.1, 0.1, 1.0};
		ofParameter<float> inputMultiplier{"input multiplier", 1.0, 0.01, 0.5};
		ofParameter<float> noiseSpeed{"noise speed", 0, 0, 1};
		ofParameter<float> normalizationFactor{"norm. factor", 1., 0.8, 1.2};
		std::vector<Octave> octaves{
			Octave{0,  20, 1.f,    1},
			Octave{1,  40, 0.5f,   1},
			Octave{2,  80, 0.25f,  1},
			Octave{3, 160, 0.125f, 1},
		};

		float noiseSeed;
		double now;

	public:

		ofParameter<int> resolution;
		ofParameterGroup paramGroup{
			"noise field",
			offsetMultiplier,
			inputMultiplier,
			noiseSpeed,
			normalizationFactor,
			octaves[0].parameters,
			octaves[1].parameters,
			octaves[2].parameters,
			octaves[3].parameters,
		};
    };
}
