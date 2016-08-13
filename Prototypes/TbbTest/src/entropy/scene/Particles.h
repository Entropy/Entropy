#pragma once

#include "ofMain.h"
#include "ofxPersistent.h"
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

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawBack();
			void drawWorld();
			void drawFront();

			void drawSkybox();
			void drawScene();

			void createRandomLights();
			void animateLights();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			bool saveState(const string & path);
			bool loadState(const string & path);

		protected:
			ofxPersistent persistent;
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
