#pragma once

#include "entropy/scene/Base.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
#include "entropy/inflation/TransitionParticles.h"
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

			void timelineBangFired(ofxTLBangEventArgs & args) override;

			void drawBackWorld() override;
			void drawFrontWorld() override;
			void drawBackOverlay() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

		protected:
			void resetWavelengths();
			void resetWavelength(size_t octave);

			bool triggerReset();
			bool triggerBigBang();
			bool triggerTransition();
			bool triggerParticles();

			void drawScene(render::Layout layout);
			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

			inflation::GPUMarchingCubes gpuMarchingCubes;
			TransitionParticles transitionParticles;
			ofBufferObject transitionParticlesPosition;
			ofShader clearParticlesVel;

			std::map<render::Layout, render::WireframeFillRenderer> renderers;

			// Noise Field
			inflation::NoiseField noiseField;

			uint64_t timeToSetIso;
			uint64_t timeToUpdate;

            //ofVbo box;

			enum State{
				PreBigBang,
				PreBigBangWobble,
				BigBang,
				Expansion,
				ExpansionTransition,
				ParticlesTransition,
			}state;

			double now = 0.0;
			double t_bigbang = 0.0;
			double t_from_bigbang = 0.0;
			double t_transition = 0.0;
			double t_from_particles = 0.0;
			float scale = 1;
			float cameraDistanceBeforeBB;
			bool octavesResetDuringTransition=false;
			bool firstCycle;
			bool needsParticlesUpdate;

			std::array<float,4> targetWavelengths;
			const std::array<ofFloatColor,4> preBigbangColors{{
				ofColor{117.f,118.f,118.f},
				ofColor{200.,200.,200.},
				ofColor(240.,127.,19.),
				ofColor(128.,9.,9.),
			}};
			std::array<ofFloatColor,4> postBigBangColors;

			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<bool> runSimulation{ "Run Simulation", true };

				ofParameter<float> bigBangDuration{ "BigBang duration", 0.25f, 0.0f, 2.f};
				ofParameter<float> preBigBangWobbleDuration{ "Pre BigBang wobble duration", 3.f, 0.0f, 5.f};
				ofParameter<float> bbFlashStart{"bigbang flash start %", 0.9, 0.f, 1.f};
				ofParameter<float> bbFlashIn{"bigbang flash in, duration sec.", 0.1, 0.f, 1.f};
				ofParameter<float> bbFlashPlateau{"bigbang flash plateau, sec.", 0.1, 0.f, 1.f};
				ofParameter<float> bbFlashOut{"bigbang flash out, duration sec.", 0.1, 0.f, 1.f};
				ofParameter<float> bbTransitionFlash{"inflation transition flash at scale.", 10.f, 2.f, 20.f};
				ofParameter<float> bbTransitionIn{"inflation transition in, duration sec.", 0.5, 0.f, 1.f};
				ofParameter<float> bbTransitionPlateau{"inflation transition plateau, sec.", 0.3, 0.f, 1.f};
				ofParameter<float> bbTransitionOut{"inflation transition out, sec.", 2, 0.f, 3.f};
				ofParameter<ofFloatColor> bbTransitionColor{"inflation transition bg color", ofFloatColor::fromHex(0x91a5a3,1)};
				ofParameter<float> transitionParticlesDuration{"transition particles in. (s)", 1, 0, 5};
				ofParameter<float> transitionBlobsOutDuration{"transition blobs out. (s)", 1, 0, 5};
				ofParameter<float> HtBB{ "Rate of expansion at bigbang", 5.f, 1.f, 100.f}; // rate of expansion
				ofParameter<float> HtPostBB{ "Rate of expansion after bigbang", 0.05f, 0.0f, 5.f}; // rate of expansion
				ofParameter<float> Ht{ "Current rate of expansion", 5.f, 0.0f, 100.f}; // rate of expansion
				ofParameter<float> hubbleWavelength{ "Hubble (min) wavelength for any octave", 4.f, 0.01f, 4.f };

				ofParameter<bool> controlCamera{ "Control Camera", false };

				struct : ofParameterGroup
				{
					ofParameter<bool> debug{ "Debug Noise", false };
					ofParameter<bool> renderBack{ "Render Back", true };
					ofParameter<bool> renderFront{ "Render Front", false };
					ofParameter<bool> boxBackRender{ "Render Box Back", false };

					PARAM_DECLARE("Render", 
						debug,
						renderBack,
						renderFront,
						boxBackRender);
				} render;

				PARAM_DECLARE("Inflation", 
					runSimulation,
					bigBangDuration,
					preBigBangWobbleDuration,
					bbFlashStart,
					bbFlashIn,
					bbFlashPlateau,
					bbFlashOut,
					bbTransitionFlash,
					bbTransitionIn,
					bbTransitionPlateau,
					bbTransitionOut,
					bbTransitionColor,
					transitionParticlesDuration,
					transitionBlobsOutDuration,
					HtBB,
					HtPostBB,
					Ht,
					hubbleWavelength,
					controlCamera,
					render);
			} parameters;
		};
    }
}
