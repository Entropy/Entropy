#pragma once

#include "entropy/geom/Sphere.h"
#include "entropy/scene/Base.h"
#include "entropy/surveys/DataSet.h"

namespace entropy
{
	namespace scene
	{
		class Surveys
			: public Base
		{
		public:
			string getName() const override
			{
				return "entropy::scene::Surveys";
			}

			Surveys();
			~Surveys();

			void init() override;
			void clear() override;

			void setup() override;
			void exit() override;

			void timelineBangFired(ofxTLBangEventArgs & args) override;

			void drawBackWorld() override;
			void drawFrontWorld() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

		protected:
			struct LayoutParameters : ofParameterGroup
			{
				ofParameter<bool> renderBoss{ "Render BOSS", true };
				ofParameter<bool> renderDes{ "Render DES", false };
				ofParameter<bool> renderVizir{ "Render ViziR", false };
				ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 100.0f };
				ofParameter<float> pointSize{ "Point Size", 8.0f, 0.01f, 32.0f };

				PARAM_DECLARE("Back", 
					renderBoss, 
					renderDes, 
					renderVizir, 
					scale, 
					pointSize);
			};

			LayoutParameters backParameters;
			LayoutParameters frontParameters;

			surveys::DataSet dataSetBoss;
			surveys::DataSet dataSetDes;
			surveys::DataSet dataSetVizir;

			geom::Sphere sphereGeom;
			ofTexture sphereTexture;
			ofShader sphereShader;

			ofShader spriteShader;
			ofTexture spriteTexture;

			void drawDataSet(LayoutParameters & parameters);

		protected:
			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			ofParameterGroup parameters;
		};
	}
}