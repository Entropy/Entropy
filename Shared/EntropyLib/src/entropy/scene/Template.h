#pragma once

#include "Base.h"

namespace entropy
{
	namespace scene
	{
		class Template
			: public Base
		{
		public:
			virtual string getName() const override 
			{
				return "entropy::scene::Template";
			}

			Template();
			~Template();

			void setup();
			void exit();

			void update();

			void drawWorld();

			void gui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<ofFloatColor> color{ "Color", ofFloatColor::crimson };
					ofxPreset::Parameter<float> radius{ "Radius", 20.0f, 0.0f, 200.0f };
					ofxPreset::Parameter<int> resolution{ "Resolution", 16, 3, 64 };

					PARAM_DECLARE("Circle", color, radius, resolution);
				} circle;

				PARAM_DECLARE("Template", circle);
			} parameters;
		};
	}
}