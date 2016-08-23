#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
#include "entropy/inflation/Constants.h"
#include "entropy/render/WireframeFillRenderer.h"

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
            entropy::render::WireframeFillRenderer renderer;

			// Noise Field
			inflation::NoiseField noiseField;

			uint64_t timeToSetIso;
            uint64_t timeToUpdate;

		protected:
            ofVbo box;
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
