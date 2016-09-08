#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/particles/ParticleSystem.h"
#include "entropy/particles/Photons.h"
#include "entropy/particles/Environment.h"

namespace entropy
{
	namespace scene
	{
		class Particles
			: public Base
		{
		public:
			static const float HALF_DIM;

		public:
			string getName() const override
			{
				return "entropy::scene::Particles";
			}

			Particles();
			~Particles();

			void init() override;
			void clear() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			void drawBackWorld() override;
			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			void drawSystem(entropy::render::Layout layout);

			bool saveState(const string & path);
			bool loadState(const string & path);

		private:
			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

			nm::ParticleSystem particleSystem;
			nm::Photons photons;
			nm::Environment::Ptr environment;
			bool debug;

			std::map<entropy::render::Layout, entropy::render::WireframeFillRenderer> renderers;

            ofShader shader;
			ofShader::TransformFeedbackSettings shaderSettings;
            ofBufferObject feedbackBuffer;
            ofVbo feedbackVbo;
            GLuint numPrimitives, numPrimitivesQuery;
            std::vector<ofLight> pointLights;

			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<string> stateFile;
                ofParameter<bool> colorsPerType{"color per type", true};
                ofParameter<bool> additiveBlending{"additive blend", true};
                ofParameter<bool> drawPhotons{"drawPhotons", true};
                ofParameter<float> ambientLight{"ambient light", 0.001, 0, 0.02};
                ofParameter<float> attenuation{"attenuation", 0.01, 0.0000001, 0.05};
				ofParameter<float> lightStrength{"light strength", 1, 0, 1};

                PARAM_DECLARE("Particles", stateFile, colorsPerType, additiveBlending, drawPhotons, ambientLight, attenuation);
			} parameters;
		};
	}
}
