#pragma once

#include "ofxRenderToolkit.h"

#include "entropy/scene/Base.h"

//#define USE_INSTANCED 1

namespace entropy
{
	namespace scene
	{
		class LightingTest
			: public Base
		{
		public:
			virtual string getName() const override
			{
				return "entropy::scene::LightingTest";
			}

			LightingTest();
			~LightingTest();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawBack();
			void drawWorld();
			void drawFront();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		public:
			void createRandomLights();
			void animateLights();

			void drawSkybox();
			void drawScene();

		protected:
			ofxRTK::util::ViewUbo viewUbo;
			ofxRTK::lighting::System lightingSystem;

			ofxRTK::pbr::CubeMapTexture radianceMap;
			ofxRTK::pbr::CubeMapTexture irradianceMap;
			ofxRTK::pbr::CubeMapTexture skyboxMap;

			ofxRTK::pbr::Material material;

			ofShader shader;
			ofShader skyboxShader;
			GLuint defaultVao;

			ofSpherePrimitive sphere;

#ifdef USE_INSTANCED
			ofVboMesh vboMesh;
			ofBufferObject bufferObject;
			ofTexture bufferTexture;
#endif

		protected:
			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofxPreset::Parameter<bool> debug{ "Debug", false, false }; 
				
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<float> exposure{ "Exposure", 1.0f, 0.01f, 10.0f };
					ofxPreset::Parameter<float> gamma{ "Gamma", 2.2f, 0.01f, 10.0f };

					PARAM_DECLARE("Camera", exposure, gamma);
				} camera;

				PARAM_DECLARE("LightingTest", debug, camera);
			} parameters;
		};
	}
}