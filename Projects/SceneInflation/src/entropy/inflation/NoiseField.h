//
//  NoiseField.hpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#pragma once

#include "ofxPreset.h"
#include "ofxVolumetrics3D.h"
#include "ofxTexture3d.h"

namespace entropy
{
	namespace inflation
	{
		struct Octave {
			Octave(size_t idx, float wavelength_, float amplitude_, const ofFloatColor & color, bool enabled = true)
				:frequency("Freq. Hz", 1/wavelength_, 0.0f, 200.0f)
				, frequencyTime("Freq time", 1/wavelength_, 0.0f, 200.f)
				, wavelength("Wavelength", wavelength_, 0.0f, 128.0f)
				, amplitude("Amplitude", amplitude_, 0.0f, 1.0f)
				, radius("Radius", 1.0f, 0.0f, 1.0f)
				, advanceTime("Advance Time", true)
				, color("Color", color)
				, enabled("Enabled", enabled)
				, parameters("Octave " + ofToString(idx),

					this->frequency,
					this->frequencyTime,
					this->wavelength,
					this->amplitude,
					this->radius,
					this->color,
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

			ofParameter<float> frequency;
			ofParameter<float> frequencyTime;
			ofParameter<float> wavelength;
			ofParameter<float> amplitude;
			ofParameter<float> radius;
			ofParameter<bool> advanceTime;
			ofParameter<ofFloatColor> color;
			ofParameter<bool> enabled;
			double now = 0;
			ofParameterGroup parameters;
			ofEventListener freqListener, waveLengthListener;
		};

		class NoiseField
		{
		public:
			NoiseField();
			void setup(ofParameter<int> & resolution);
			void update();
			void draw(float threshold);

			int getNumScales();
			void numScalesChanged(int& numScales);

			float getValue(int x, int y, int z);
			ofxTexture3d & getTexture() {
				return volumeTex;
			}

			ofParameter<float> noiseSpeed{ "Noise Speed", 0.0f, 0.0f, 5.0f };
			ofParameter<float> speedFactor{ "Noise Speed Factor", 1.0f, 0.0f, 1.0f };
			ofParameter<float> normalizationFactor{ "Norm. Factor", 1.0f, 0.8f, 1.2f };
			ofParameter<float> oscillate{ "Oscillate", 0.0f, 0.f, 1.f };
			ofParameter<float> oscillateSpeed{ "Oscillate speed", 1.0f, 0.f, 10.f };
			ofParameter<float> oscillateSpatialFreq{"Oscillate spatial freq.", 64, 1, 1000};
			ofParameter<bool> fillEdges{ "Fill Edges", false };
			ofParameter<float> scale{"Scale", 1.f, 0.0f, 1000.f};
			std::vector<Octave> octaves;

		private:

			float noiseSeed;
			ofShader noiseComputeShader;
			ofxVolumetrics3D volumetrics;
			ofxTexture3d volumeTex;
			std::vector<ofFloatColor> volumeValues;
			ofShader volumeShader;
			ofEventListener sphericalClipListener, fillEdgesListener, resolutionListener;
			void allocateVolumeTexture();
            ofShader::Settings shaderSettings;
			float now;

		public:
			ofParameter<int> resolution;
			ofParameterGroup parameters{
				"Noise Field",
				noiseSpeed,
				normalizationFactor,
				fillEdges,
				oscillate,
				oscillateSpeed,
				oscillateSpatialFreq,
			};
		};
	}
}
