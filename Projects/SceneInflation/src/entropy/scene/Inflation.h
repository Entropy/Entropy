#pragma once

#include "ofMain.h"

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

			void setup() override;
			void exit() override;

			//void resizeBack(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawBackWorld() override;
			void drawFrontOverlay() override;

			//bool postProcessBack(const ofTexture & srcTexture, const ofFbo & dstFbo) override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			double now;

			inflation::GPUMarchingCubes gpuMarchingCubes;

			// Noise Field
			inflation::NoiseField noiseField;


			// GUI
			//bool guiVisible = true;
			uint64_t timeToSetIso;
			uint64_t timeToUpdate;

			/*
			// Post Effects
			ofFbo fboPost[2];

			ofShader shaderBright;
			ofShader blurV;
			ofShader blurH;
			ofShader tonemap;
			*/

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<bool> runSimulation{ "Run Simulation", true };

				struct : ofParameterGroup
				{
					ofParameter<bool> debug{ "Debug noise", false };
					ofParameter<bool> additiveBlending{ "Additive Blending", false };

					PARAM_DECLARE("Render", 
						debug,
						additiveBlending );
				} render;

				PARAM_DECLARE("Inflation", runSimulation, render);
			} parameters;
		};
    }
}
