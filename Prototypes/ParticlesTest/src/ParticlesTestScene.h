#pragma once

#include "entropy/scene/Base.h"
#include "ParticleSystem.h"
#include "ofxPersistent/ofxPersistent.h"

namespace entropy
{
	namespace scene
	{
		class ParticlesTestScene
			: public Base
		{
		public:
			static const unsigned NUM_LIGHTS = 2;

			virtual string getName() const override 
			{
				return "entropy::scene::ParticlesTestScene";
			}

			ParticlesTestScene();
			~ParticlesTestScene();

			void setup();
			void exit();

			void update(double & dt);

			void drawBack();
			void drawWorld();
			void drawFront();

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			void deleteRandomParticle();

		protected:
			// particles
			ParticleSystem particleSystem;
			ofShader particleShader;

			ofEasyCam cam;

			// lighting
			ofVec3f lightPosns[NUM_LIGHTS];
			ofFloatColor lightCols[NUM_LIGHTS];
			float lightIntensities[NUM_LIGHTS];
			float lightRadiuses[NUM_LIGHTS];
			float roughness;

			ofxPersistent persistent;

			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<ofFloatColor> color{ "Color", ofFloatColor::crimson };
					ofxPreset::Parameter<bool> filled{ "Filled", false, true };
					ofxPreset::Parameter<float> radius{ "Radius", 20.0f, 0.0f, 200.0f };
					ofxPreset::Parameter<int> resolution{ "Resolution", 16, 3, 64 };

					PARAM_DECLARE("Sphere", color, filled, radius, resolution);
				} sphere;

				PARAM_DECLARE("Template", sphere);
				
			} parameters;
		};
	}
}