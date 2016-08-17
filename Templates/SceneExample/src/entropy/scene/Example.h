#pragma once

#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Example
			: public Base
		{
		public:
			string getName() const override 
			{
				return "entropy::scene::Example";
			}

			Example();
			~Example();

			void setup() override;
			void exit() override;
			void resize(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawBackBase() override;
			void drawBackWorld() override;
			void drawBackOverlay() override;

			void drawFrontBase() override;
			void drawFrontWorld() override;
			void drawFrontOverlay() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void serialize(nlohmann::json & json) override;
			void deserialize(const nlohmann::json & json) override;

		protected:
			ofBoxPrimitive sphere;
			ofLight light;
			ofMaterial material;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				struct : ofParameterGroup
				{
					ofParameter<ofFloatColor> color{ "Color", ofFloatColor::crimson };
					ofParameter<bool> filled{ "Filled", false };
					ofParameter<float> radius{ "Radius", 20.0f, 0.0f, 200.0f };
					ofParameter<int> resolution{ "Resolution", 16, 3, 64 };

					PARAM_DECLARE("Sphere", color, filled, radius, resolution);
				} sphere;

				PARAM_DECLARE("Template", sphere);
			} parameters;
		};
	}
}