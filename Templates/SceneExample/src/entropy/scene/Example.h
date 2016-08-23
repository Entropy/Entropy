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

			render::Layout getGridLayout();
			void clearGrid();
			void updateGrid(render::Layout layout);
			void drawGrid();

			ofVboMesh pointsMesh;
			ofVboMesh horizontalMesh;
			ofVboMesh verticalMesh;
			ofVboMesh crossMesh;

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

				struct : ofParameterGroup
				{
					ofParameter<int> layout{ "Layout", static_cast<int>(render::Layout::Front), static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Front) };
					ofParameter<int> size{ "Size", 20, 1, 200 };
					ofParameter<bool> centerPoints{ "Center Points", true };
					ofParameter<bool> horizontalLines{ "Horizontal Lines", true };
					ofParameter<bool> verticalLines{ "Vertical Lines", true };
					ofParameter<bool> crossLines{ "Cross Lines", false };

					PARAM_DECLARE("Grid", size, centerPoints, horizontalLines, verticalLines, crossLines);
				} grid;

				PARAM_DECLARE("Example", box, grid);
			} parameters;
		};
	}
}