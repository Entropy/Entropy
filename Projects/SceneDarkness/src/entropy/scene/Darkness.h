#pragma once

#include "entropy/scene/Base.h"
#include "entropy/darkness/DataSet.h"

namespace entropy
{
	namespace scene
	{
		class Darkness
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Darkness";
			}

			Darkness();
			~Darkness();

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

			entropy::darkness::DataSet dataSetBoss;
			entropy::darkness::DataSet dataSetDes;

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

				struct : ofParameterGroup
				{
					ofParameter<int> fragments{ "Fragments", 1, 0, 10 };
					ofParameter<float> minDistance{ "Min Distance", 0.0f, 0.0f, 1.0f };
					ofParameter<float> maxDistance{ "Max Distance", 0.5f, 0.0f, 1.0f };

					PARAM_DECLARE("BOSS", fragments, minDistance, maxDistance);
				} boss;

				struct : ofParameterGroup
				{
					ofParameter<int> fragments{ "Fragments ", 1, 0, 20 };
					ofParameter<float> minDistance{ "Min Distance ", 0.0f, 0.0f, 1.0f };
					ofParameter<float> maxDistance{ "Max Distance ", 0.5f, 0.0f, 1.0f };

					PARAM_DECLARE("DES", fragments, minDistance, maxDistance);
				} des;

				PARAM_DECLARE("Darkness", render, boss, des);
			} parameters;
		};
	}
}