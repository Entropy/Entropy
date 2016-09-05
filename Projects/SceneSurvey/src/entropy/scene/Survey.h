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

			void init() override;
			void clear() override;

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
				ofParameter<bool> renderVizir{ "Render ViziR", false };
				ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 20.0f };
				ofParameter<float> pointSize{ "Point Size", 8.0f, 0.01f, 10.0f };

				PARAM_DECLARE("Back", 
					renderBoss, 
					renderDes, 
					renderVizir, 
					scale, 
					pointSize);
			};

			LayoutParameters backParameters;
			LayoutParameters frontParameters;

			survey::DataSet dataSetBoss;
			survey::DataSet dataSetDes;
			survey::DataSet dataSetVizir;

			ofVboMesh galaxyQuad;

			ofShader spriteShader;
			ofTexture texture;

			void drawDataSet(LayoutParameters & parameters);

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				struct : ofParameterGroup
				{
					ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };
					ofParameter<float> scale{ "Scale", 1.0f, 0.0f, 10000.0f };
					ofParameter<glm::vec3> orientation{ "Orientation", glm::vec3(0.0f), glm::vec3(-180.0f), glm::vec3(180.0f) };

					PARAM_DECLARE("Galaxy", 
						alpha, 
						scale, 
						orientation);
				} galaxy;

				PARAM_DECLARE("Survey", galaxy);
			} parameters;
		};
	}
}