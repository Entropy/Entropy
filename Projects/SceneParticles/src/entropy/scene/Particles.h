#pragma once

#include "ofMain.h"

#include "entropy/scene/Base.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/particles/ParticleSystem.h"
#include "entropy/particles/Photons.h"
#include "entropy/particles/Environment.h"
#include "entropy/particles/ViewUbo.h"

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

			void setup() override;

			void update(double dt) override;

			void drawBackWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			void drawSkybox();

			bool saveState(const string & path);
			bool loadState(const string & path);

        private:
			nm::ParticleSystem particleSystem;
			nm::Photons photons;
			nm::Environment::Ptr environment;


            void compileShader();
			bool debug;

            entropy::render::WireframeFillRenderer renderer;

            ofShader shader;
            ofBufferObject feedbackBuffer;
            ofVbo feedbackVbo;
            GLuint numPrimitives, numPrimitivesQuery;
            ofxRenderToolkit::util::ViewUbo viewUbo;
            std::vector<ofLight> pointLights;

			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

            ofParameter<bool> colorsPerType{"color per type", true};
            ofParameter<bool> additiveBlending{"additive blend", true};
            ofEventListener colorsPerTypeListener;

			struct : BaseParameters
			{
				ofParameter<string> stateFile;

				PARAM_DECLARE("Particles", stateFile);
			} parameters;
		};
	}
}
