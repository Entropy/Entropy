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
        Octave(size_t idx, float frequency_, float amplitude_)
            :frequency("freq. hz", frequency_, 0, 200)
            ,wavelength("wavelength", 1.f/frequency_, 0, 128)
            ,amplitude("ampl. norm", amplitude_, 0, 1)
            ,radius("radius", 1, 0, 1)
            ,advanceTime("advance time", true)
		    ,enabled("enabled", true)
		    ,parameters("octave " + ofToString(idx),
                        this->frequency,
                        this->wavelength,
                        this->amplitude,
                        this->radius,
                        this->enabled,
                        this->advanceTime){

            auto wavelength(this->wavelength);
            freqListener = this->frequency.newListener([wavelength](float & freq) mutable{
                wavelength = 1.f/freq;
            });

            auto frequency(this->frequency);
            waveLengthListener = this->wavelength.newListener([frequency](float & l) mutable{
                frequency = 1.f/l;
            });
        }

		ofParameter<float> frequency;
        ofParameter<float> wavelength;
		ofParameter<float> amplitude;
        ofParameter<float> radius;
        ofParameter<bool> advanceTime;	
		ofParameter<bool> enabled;
        double now=0;
        ofParameterGroup parameters;
        ofEventListener freqListener, waveLengthListener;
	};

    class NoiseField
    {
    public:
        NoiseField();
        void setup(ofParameter<int> & resolution);
        void update(bool inflation, float scale);
		void draw(float threshold);

        int getNumScales();
        void numScalesChanged(int& numScales);

        float getValue(int x, int y, int z);
		ofxTexture3d & getTexture(){
			return volumeTex;
		}


    private:
        ofParameter<float> noiseSpeed{"noise speed", 0, 0, 5};
		ofParameter<float> normalizationFactor{"norm. factor", 1., 0.8, 1.2};
        ofParameter<float> fadeAt{"fadeAt", 0.8, 0, 1};
        ofParameter<bool> sphericalClip{"spherical clip", false};
        ofParameter<bool> fillEdges{"fill edges", false};
        std::vector<Octave> octaves;

		float noiseSeed;
		ofShader noiseComputeShader;
		ofxVolumetrics3D volumetrics;
		ofxTexture3d volumeTex;
        std::vector<ofFloatColor> volumeValues;
		ofShader volumeShader;
        ofEventListener sphericalClipListener, fillEdgesListener, resolutionListener;

        void compileComputeShader();
        void allocateVolumeTexture();

	public:

		ofParameter<int> resolution;
        ofParameterGroup parameters{
            "noise field",
			noiseSpeed,
			normalizationFactor,
            fadeAt,
            sphericalClip,
            fillEdges,
		};
    };
}
