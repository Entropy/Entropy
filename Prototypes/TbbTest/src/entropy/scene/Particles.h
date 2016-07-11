#pragma once

#include "ofMain.h"
#include "ofxPersistent.h"

#include "entropy/scene/Base.h"
#include "entropy/particles/ParticleSystem.h"
#include "entropy/particles/Photons.h"

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
			virtual string getName() const override
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

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			ofxPersistent persistent;
			nm::ParticleSystem particleSystem;
			nm::Photons photons;
			
		protected:
			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				//struct : ofParameterGroup
				//{
				//	ofxPreset::Parameter<ofFloatColor> color{ "Color", ofFloatColor::crimson };
				//	ofxPreset::Parameter<bool> filled{ "Filled", false, false };
				//	ofxPreset::Parameter<float> radius{ "Radius", 20.0f, 0.0f, 200.0f };
				//	ofxPreset::Parameter<int> resolution{ "Resolution", 16, 3, 64 };

				//	PARAM_DECLARE("Sphere", color, filled, radius, resolution);
				//} sphere;

				//PARAM_DECLARE("Particles", sphere);
			} parameters;
		};
	}
}
