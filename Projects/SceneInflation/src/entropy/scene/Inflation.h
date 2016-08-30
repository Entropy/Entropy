#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
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

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			void drawBackWorld() override;
			void drawFrontOverlay() override;
			void drawBackOverlay() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;


			inflation::GPUMarchingCubes gpuMarchingCubes;
            render::WireframeFillRenderer renderer;

			// Noise Field
			inflation::NoiseField noiseField;

			uint64_t timeToSetIso;
            uint64_t timeToUpdate;

			void resetWavelengths();
			void updateOctaves(double dt);
            //ofVbo box;

			enum State{
				PreBigBang,
				PreBigBangWobbly,
				BigBang,
				Expansion,
				ExpansionFadeOut,
				ExpansionFadeIn,
			}state;

			double now = 0.0;
			double t_bigbang = 0.0;
			double t_from_bigbang = 0.0;
			double t_fade_out;
			float fadeEdge0 = 0.0, fadeEdge1 = 0.0;
			float originalFillAlpha;
			float originalWireframeAlpha;
			size_t cycle = 0;

			std::array<float,8> targetWavelengths;
			std::array<float,8> targetAmplitudes;

			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<bool> runSimulation{ "Run Simulation", true };
				ofParameter<float> wobblyDuration{ "Wobbly duration", 2.5f, 0.0f, 5.f};
				ofParameter<float> bigBangDuration{ "BigBang duration", 0.175f, 0.0f, 2.f};
				ofParameter<float> expansionFractalFade{ "Fractal fade (Begin scale)", 20.f, 1.f, 50.f};
				ofParameter<float> expansionFractalFadeDuration{ "Fractal fade out duration", 2.5f, 0.2f, 3.f};
				ofParameter<float> Ht1{"Expansion rate 1nd cycle", 20.f, 0.1f, 10.f};
				ofParameter<float> Ht2{"Expansion rate 2nd cycle", 0.2f, 0.1f, 10.f};
				ofParameter<float> Ht3{"Min expansion rate", 0.01f, 0.1f, 10.f};
				ofParameter<float> scale{"Inflation", 1.f, 1.f, 35.f};
				ofParameter<ofFloatColor> fadeOutBgColor{"Fade bg color", ofFloatColor::fromHex(0x91a5a3,1)};
				ofParameter<float> currentFractalFadeScale{ "Current Fractal fade (Begin scale)", 15.f, 1.f, 50.f};
				ofParameter<float> Ht{"Current expansion rate", 20.f, 0.f, 1.f}; // rate of expansion
				ofParameter<float> blobsFade{"Background fade", 0.f, 0.f, 1.f};
				ofParameter<float> hubbleFreq{"Hubble frequency", 0, 0, 10};

				struct : ofParameterGroup
				{
					ofParameter<bool> debug{ "Debug Noise", false };
					ofParameter<bool> additiveBlending{ "Additive Blending", false };
					ofParameter<bool> drawBoxInRenderer{ "Draw Box In Renderer", false };

					PARAM_DECLARE("Render", 
						debug,
						additiveBlending,
						drawBoxInRenderer);
				} render;

				ofParameterGroup parameters{"Inflation",
					runSimulation,
					wobblyDuration,
					bigBangDuration,
					expansionFractalFade,
					expansionFractalFadeDuration,
					Ht1,
					Ht2,
					Ht3,
					fadeOutBgColor,
					scale,
					currentFractalFadeScale,
					Ht,
					blobsFade};
				PARAM_DECLARE("Inflation", parameters);
			} parameters;


			std::vector<std::deque<float>> timeSeries{8};
		};
    }
}
