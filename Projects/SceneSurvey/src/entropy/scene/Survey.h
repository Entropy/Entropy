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

			void setup() override;
			void exit() override;

			void drawBackWorld() override;
			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

		protected:
			struct LayoutParameters : ofParameterGroup
			{
				ofParameter<bool> renderBoss{ "Render BOSS", true };
				ofParameter<bool> renderDes{ "Render DES", false };
				ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 20.0f };
				ofParameter<float> pointSize{ "Point Size", 8.0f, 0.01f, 10.0f };

				PARAM_DECLARE("Back", renderBoss, renderDes, scale, pointSize);
			};

			LayoutParameters backParameters;
			LayoutParameters frontParameters;

			survey::DataSet dataSetBoss;
			survey::DataSet dataSetDes;

			ofShader spriteShader;
			ofTexture texture;

			void drawDataSet(LayoutParameters & parameters);

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			BaseParameters parameters;
		};
	}
}