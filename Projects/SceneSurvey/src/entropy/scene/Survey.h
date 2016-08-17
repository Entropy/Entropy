#pragma once

#include "entropy/scene/Base.h"
#include "entropy/survey/DataSet.h"

namespace entropy
{
	namespace scene
	{
		class Survey
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Survey";
			}

			Survey();
			~Survey();

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
			ofShader spriteShader;
			ofTexture texture;

			entropy::survey::DataSet dataSetBoss;
			entropy::survey::DataSet dataSetDes;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				struct : ofParameterGroup
				{
					ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 20.0f };
					ofParameter<float> pointSize{ "Point Size", 8.0f, 0.01f, 32.0f };
					ofParameter<bool> useSprites{ "Use Sprites", true };

					PARAM_DECLARE("Render", scale, pointSize, useSprites);
				} render;

				PARAM_DECLARE("Survey", render);
			} parameters;
		};
	}
}