//
//  NoiseField.hpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxVolumetrics3D.h"
#include "ofxTexture3d.h"

namespace entropy
{
	namespace inflation
	{
		struct Octave {
			Octave(size_t idx, float frequency_, float amplitude_)
				:frequency("Freq. Hz", frequency_, 0.0f, 200.0f)
				, wavelength("Wavelength", 1.f / frequency_, 0.0f, 128.0f)
				, amplitude("Amplitude", amplitude_, 0.0f, 1.0f)
				, radius("Radius", 1.0f, 0.0f, 1.0f)
				, advanceTime("Advance Time", true, false)
				, enabled("Enabled", true, false)
				, parameters("Octave " + ofToString(idx),
					this->frequency,
					this->wavelength,
					this->amplitude,
					this->radius,
					this->enabled,
					this->advanceTime) {

				// Give every parameter a unique name (or else imgui gets confused)
				auto idxStr = ofToString(idx);
				for (auto & param : parameters) {
					param->setName(param->getName() + " " + idxStr);
				}

				auto wavelength(this->wavelength);
				freqListener = this->frequency.newListener([wavelength](float & freq) mutable {
					wavelength.setWithoutEventNotifications(1.f / freq);
				});

				auto frequency(this->frequency);
				waveLengthListener = this->wavelength.newListener([frequency](float & l) mutable {
					frequency = 1.f / l;
				});
			}

			ofxPreset::Parameter<float> frequency;
			ofxPreset::Parameter<float> wavelength;
			ofxPreset::Parameter<float> amplitude;
			ofxPreset::Parameter<float> radius;
			ofxPreset::Parameter<bool> advanceTime;
			ofxPreset::Parameter<bool> enabled;
			double now = 0;
			ofParameterGroup parameters;
			ofEventListener freqListener, waveLengthListener;
		};

		class NoiseField
		{
		public:
			NoiseField();
			void setup(ofParameter<int> & resolution);
			void update(bool inflation);
			void draw(float threshold);

			int getNumScales();
			void numScalesChanged(int& numScales);

			float getValue(int x, int y, int z);
			ofxTexture3d & getTexture() {
				return volumeTex;
			}


		private:
			ofxPreset::Parameter<float> noiseSpeed{ "Noise Speed", 0.0f, 0.0f, 5.0f };
			ofxPreset::Parameter<float> normalizationFactor{ "Norm. Factor", 1.0f, 0.8f, 1.2f };
			ofxPreset::Parameter<float> fadeAt{ "Fade At", 0.8f, 0.0f, 1.0f };
			ofxPreset::Parameter<bool> sphericalClip{ "Spherical Clip", false, false };
			ofxPreset::Parameter<bool> fillEdges{ "Fill Edges", false, false };
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

			ofxPreset::Parameter<int> resolution;
			ofParameterGroup parameters{
				"Noise Field",
				noiseSpeed,
				normalizationFactor,
				fadeAt,
				sphericalClip,
				fillEdges,
			};
		};
	}
}
