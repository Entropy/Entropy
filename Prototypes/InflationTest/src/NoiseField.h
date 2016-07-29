//
//  NoiseField.hpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxVolumetrics3D.h"
#include "ofxTexture3d.h"

namespace entropy
{
	struct Octave{
		Octave(size_t idx, float frequency, float amplitude, float radius)
		    :frequency("freq. hz", frequency, 0, 200)
		    ,amplitude("ampl. norm", amplitude, 0, 1)
		    ,radius("radius", radius, 0, 1)
            ,advanceTime("advance time", true)
		    ,enabled("enabled", true)
		    ,parameters("octave " + ofToString(idx),
                        this->frequency,
                        this->amplitude,
                        this->radius,
                        this->enabled,
                        this->advanceTime){}

		ofParameter<float> frequency;
		ofParameter<float> amplitude;
        ofParameter<float> radius;
        ofParameter<bool> advanceTime;	
		ofParameter<bool> enabled;
        double now=0;
        ofParameterGroup parameters;
	};

    class NoiseField
    {
    public:
        void setup(ofParameter<int> & resolution);
        void update(bool inflation);
		void draw(float threshold);

        int getNumScales();
        void numScalesChanged(int& numScales);

        float getValue(int x, int y, int z);
		ofxTexture3d & getTexture(){
			return volumeTex;
		}


	private:
		ofParameter<float> offsetMultiplier{"offset multiplier", 0.1, 0.1, 1.0};
		ofParameter<float> inputMultiplier{"input multiplier", 1.0, 0.01, 0.5};
		ofParameter<float> noiseSpeed{"noise speed", 0, 0, 1};
		ofParameter<float> normalizationFactor{"norm. factor", 1., 0.8, 1.2};
        ofParameter<float> fadeAt{"fadeAt", 0.8, 0, 1};
        ofParameter<bool> sphericalClip{"spherical clip", false};
        ofParameter<bool> fillEdges{"fill edges", false};
		std::vector<Octave> octaves{
			Octave{0,  20, 1.f,    1},
			Octave{1,  40, 0.5f,   1},
			Octave{2,  80, 0.25f,  1},
			Octave{3, 160, 0.125f, 1},
		};

		float noiseSeed;
		ofShader noiseComputeShader;
		ofxVolumetrics3D volumetrics;
		ofxTexture3d volumeTex;
		ofShader volumeShader;
        ofEventListener sphericalClipListener, fillEdgesListener, resolutionListener;

        void compileComputeShader();
        void allocateVolumeTexture();

	public:

		ofParameter<int> resolution;
        ofParameterGroup parameters{
			"noise field",
			offsetMultiplier,
			inputMultiplier,
			noiseSpeed,
			normalizationFactor,
            fadeAt,
            sphericalClip,
            fillEdges,
			octaves[0].parameters,
			octaves[1].parameters,
			octaves[2].parameters,
			octaves[3].parameters,
		};
    };
}
