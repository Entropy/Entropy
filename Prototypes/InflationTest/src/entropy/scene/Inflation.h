#pragma once

#include "ofMain.h"
#include "ofxTextureRecorder.h"

#include "entropy/scene/Base.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
#include "entropy/inflation/Constants.h"

namespace entropy
{
    namespace scene
    {
		class Inflation
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Inflation";
			}

			Inflation();
			~Inflation();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawWorld();
			void drawFront();

			bool postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo) override;

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			double now;

			inflation::GPUMarchingCubes gpuMarchingCubes;

			// Noise Field
			inflation::NoiseField noiseField;


			// GUI
			//bool guiVisible = true;
			uint64_t timeToSetIso;
			uint64_t timeToUpdate;


			// Post Effects
			ofFbo fboPost[2];

			ofShader shaderBright;
			ofShader blurV;
			ofShader blurH;
			ofShader tonemap;

			ofxTextureRecorder saverThread;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofxPreset::Parameter<bool> runSimulation{ "Run Simulation", true, false };
				
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<float> scale{ "Scale", 1.0f, 1.0f, 100.0f };
					ofxPreset::Parameter<int> resolution{ "Resolution", 1, 1, 512 };
					ofxPreset::Parameter<float> threshold{ "Threshold", 0.345f, 0.0f, 1.0f };
					ofxPreset::Parameter<bool> inflation{ "Inflation", false, false };

					PARAM_DECLARE("Marching Cubes", scale, resolution, threshold, inflation);
				} marchingCubes;

				struct : ofParameterGroup
				{
					ofxPreset::Parameter<bool> debug{ "Debug", false, false };
					ofxPreset::Parameter<bool> drawGrid{ "Draw Grid", true, false };
					ofxPreset::Parameter<bool> wireframe{ "Wireframe", true, false };
					ofxPreset::Parameter<bool> shadeNormals{ "Shade Normals", false, false };
					ofxPreset::Parameter<bool> additiveBlending{ "Additive Blending", false, false };

					struct : ofParameterGroup
					{
						ofxPreset::Parameter<bool> enabled{ "Enabled", true, false };
						ofxPreset::Parameter<float> brightnessThreshold{ "Brightness Threshold", 1.0f, 0.5f, 3.0f };
						ofxPreset::Parameter<float> sigma{ "Sigma", 0.9f, 0.5f, 18.0f };
						ofxPreset::Parameter<float> contrast{ "Contrast",1.0f, 0.5f, 1.5f };
						ofxPreset::Parameter<float> brightness{ "Brightness", 0.0f, -1.0f, 1.0f };
						ofxPreset::Parameter<int> tonemapType{ "Tonemap Type", 0, 0, 5, true };

						PARAM_DECLARE("Bloom", enabled, brightnessThreshold, sigma, contrast, brightness, tonemapType);
					} bloom;

					PARAM_DECLARE("Render", debug, drawGrid, wireframe, shadeNormals, additiveBlending, bloom);
				} render;

				ofxPreset::Parameter<bool> record{ "Record", false, false };

				PARAM_DECLARE("Inflation", runSimulation, marchingCubes, render, record);
			} parameters;
		};
    }
}
