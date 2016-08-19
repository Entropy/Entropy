#pragma once

#include "ofMain.h"
#include "ofxRenderToolkit.h"

#include "entropy/scene/Base.h"
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

			void setup() override;

			void update(double dt) override;

			void drawBackWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

			void drawSkybox();

			bool saveState(const string & path);
			bool loadState(const string & path);

		protected:
			nm::ParticleSystem particleSystem;
			nm::Photons photons;
			nm::Environment::Ptr environment;
			
		protected:
			bool debug;

			ofxRTK::util::ViewUbo viewUbo;
			ofxRTK::lighting::System lightingSystem;

			ofxRTK::pbr::CubeMapTexture radianceMap;
			ofxRTK::pbr::CubeMapTexture irradianceMap;
			ofxRTK::pbr::CubeMapTexture skyboxMap;

			ofxRTK::pbr::Material material;

			ofShader shader;
			ofShader skyboxShader;
			GLuint defaultVao;

			float exposure;
			float gamma;

			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> stateFile;

				PARAM_DECLARE("Particles", stateFile);
			} parameters;
		};
	}
}
