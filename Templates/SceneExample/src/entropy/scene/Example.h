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
			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

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
			void drawScene(bool filled);
			void drawOverlay(bool filled, render::Layout layout);
			
			ofBoxPrimitive box;
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
					ofParameter<float> size{ "Size", 20.0f, 0.0f, 200.0f };

					PARAM_DECLARE("Box", color, size);
				} box;

				PARAM_DECLARE("Example", box);
			} parameters;
		};
	}
}