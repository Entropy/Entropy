#include "Calibrate.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Calibrate::Calibrate()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Calibrate::~Calibrate()
		{}

		//--------------------------------------------------------------
		/// Initialize and preload persistent stuff here.
		void Calibrate::init()
		{}

		//--------------------------------------------------------------
		/// Delete any persistent stuff initialized in the constructor.
		void Calibrate::clear()
		{}

		//--------------------------------------------------------------
		/// Wake up your scene for pre-roll. You can set preset specific settings in deserialize().
		void Calibrate::setup()
		{
			ofEnableLighting();
		}
		
		//--------------------------------------------------------------
		/// Clean up your scene to go idle.
		void Calibrate::exit()
		{
			
		}

		//--------------------------------------------------------------
		/// Resize your content here. 
		/// Note that this is not the window size but the canvas size.
		void Calibrate::resizeBack(ofResizeEventArgs & args)
		{
			if (this->getGridLayout() == render::Layout::Back)
			{
				this->clearGrid();
			}
		}

		//--------------------------------------------------------------
		/// Resize your content here. 
		/// Note that this is not the window size but the canvas size.
		void Calibrate::resizeFront(ofResizeEventArgs & args)
		{
			if (this->getGridLayout() == render::Layout::Front)
			{
				this->clearGrid();
			}
		}

		//--------------------------------------------------------------
		/// Update your data here, once per frame.
		void Calibrate::update(double dt)
		{
			this->updateGrid(this->getGridLayout());
		}

		//--------------------------------------------------------------
		/// Draw 2D elements in the background here.
		void Calibrate::drawBackBase()
		{
			if (this->parameters.border.drawBack)
			{
				this->drawBorder(render::Layout::Back);
			}
		}
		
		//--------------------------------------------------------------
		/// Draw 3D elements here.
		void Calibrate::drawBackWorld()
		{

		}

		//--------------------------------------------------------------
		/// Draw 2D elements in the foreground here. Note that these won't be post-processed.
		void Calibrate::drawBackOverlay()
		{
			if (this->getGridLayout() == render::Layout::Back)
			{
				this->drawGrid();
			}
		}

		//--------------------------------------------------------------
		/// Draw 2D elements in the background here.
		void Calibrate::drawFrontBase()
		{
			if (this->parameters.border.drawFront)
			{
				this->drawBorder(render::Layout::Front);
			}
		}

		//--------------------------------------------------------------
		/// Draw 3D elements here.
		void Calibrate::drawFrontWorld()
		{

		}

		//--------------------------------------------------------------
		/// Draw 2D elements in the foreground here. Note that these won't be post-processed.
		void Calibrate::drawFrontOverlay()
		{
			if (this->getGridLayout() == render::Layout::Front)
			{
				this->drawGrid();
			}
		}

		//--------------------------------------------------------------
		void Calibrate::drawBorder(render::Layout layout)
		{
			ofPushStyle();
			{
				const auto borderSize = this->parameters.border.size;
				const auto canvasWidth = GetCanvasWidth(layout);
				const auto canvasHeight = GetCanvasHeight(layout);

				ofSetColor(255, 0, 0, 128);
				ofDrawRectangle(0.0f, 0.0f, canvasWidth, borderSize);
				ofSetColor(0, 255, 0, 128);
				ofDrawRectangle(0.0f, canvasHeight - borderSize, canvasWidth, borderSize);
				ofSetColor(0, 0, 255, 128);
				ofDrawRectangle(0.0f, 0.0f, borderSize, canvasHeight);
				ofSetColor(0, 255, 255, 128);
				ofDrawRectangle(canvasWidth - borderSize, 0.0f, borderSize, canvasHeight);
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		render::Layout Calibrate::getGridLayout()
		{
			return static_cast<render::Layout>(this->parameters.grid.layout.get());
		}

		//--------------------------------------------------------------
		void Calibrate::clearGrid()
		{
			this->pointsMesh.clear();
			this->horizontalMesh.clear();
			this->verticalMesh.clear();
			this->crossMesh.clear();
		}

		//--------------------------------------------------------------
		void Calibrate::updateGrid(render::Layout layout)
		{
			if (this->parameters.grid.centerPoints && this->pointsMesh.getNumVertices() == 0)
			{
				// Rebuild points.
				const auto numPointsHorz = GetCanvasWidth(layout) / this->parameters.grid.size;
				const auto numPointsVert = GetCanvasHeight(layout) / this->parameters.grid.size;
				this->pointsMesh.setMode(OF_PRIMITIVE_POINTS);
				for (int j = 0; j < numPointsVert; ++j)
				{
					for (int i = 0; i < numPointsHorz; ++i)
					{
						this->pointsMesh.addVertex(glm::vec3(this->parameters.grid.size / 2 + i * this->parameters.grid.size, this->parameters.grid.size / 2 + j * this->parameters.grid.size, 0));
					}
				}
			}

			if (this->parameters.grid.horizontalLines && this->horizontalMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto numLines = GetCanvasHeight(layout) / this->parameters.grid.size + 1;
				this->horizontalMesh.setMode(OF_PRIMITIVE_LINES);
				for (int i = 0; i < numLines; ++i)
				{
					this->horizontalMesh.addVertex(glm::vec3(0, i * this->parameters.grid.size, 0));
					this->horizontalMesh.addVertex(glm::vec3(GetCanvasWidth(layout), i * this->parameters.grid.size, 0));
					if (i % 4 == 0)
					{
						this->horizontalMesh.addColor(ofFloatColor::red);
						this->horizontalMesh.addColor(ofFloatColor::red);
					}
					else
					{
						this->horizontalMesh.addColor(ofFloatColor::yellow);
						this->horizontalMesh.addColor(ofFloatColor::yellow);
					}
				}
			}

			if (this->parameters.grid.verticalLines && this->verticalMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto numLines = GetCanvasWidth(layout) / this->parameters.grid.size + 1;
				this->verticalMesh.setMode(OF_PRIMITIVE_LINES);
				for (int i = 0; i < numLines; ++i)
				{
					this->verticalMesh.addVertex(glm::vec3(i * this->parameters.grid.size, 0, 0));
					this->verticalMesh.addVertex(glm::vec3(i * this->parameters.grid.size, GetCanvasHeight(layout), 0));
					if (i % 4 == 0)
					{
						this->verticalMesh.addColor(ofFloatColor::green);
						this->verticalMesh.addColor(ofFloatColor::green);
					}
					else
					{
						this->verticalMesh.addColor(ofFloatColor::yellow);
						this->verticalMesh.addColor(ofFloatColor::yellow);
					}
				}
			}

			if (this->parameters.grid.crossLines && this->crossMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto numLines = GetCanvasWidth(layout) / this->parameters.grid.size + 1;
				this->crossMesh.setMode(OF_PRIMITIVE_LINES);
				this->crossMesh.addVertex(glm::vec3(0, 0, 0));
				this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), GetCanvasHeight(layout), 0));
				this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), 0, 0));
				this->crossMesh.addVertex(glm::vec3(0, GetCanvasHeight(layout), 0));
				this->crossMesh.addColor(ofFloatColor::magenta);
				this->crossMesh.addColor(ofFloatColor::magenta);
				this->crossMesh.addColor(ofFloatColor::magenta);
				this->crossMesh.addColor(ofFloatColor::magenta);
			}
		}

		//--------------------------------------------------------------
		void Calibrate::drawGrid()
		{
			if (this->parameters.grid.centerPoints)
			{
				this->pointsMesh.draw();
			}

			if (this->parameters.grid.horizontalLines)
			{
				this->horizontalMesh.draw();
			}

			if (this->parameters.grid.verticalLines)
			{
				this->verticalMesh.draw();
			}

			if (this->parameters.grid.crossLines)
			{
				this->crossMesh.draw();
			}
		}

		//--------------------------------------------------------------
		/// Add Scene specific gui windows here.
		void Calibrate::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				// Add parameters manually.
				if (ImGui::CollapsingHeader(this->parameters.grid.getName().c_str(), nullptr, true, true))
				{
					static vector<string> labels{ "Back", "Front" };
					if (ofxPreset::Gui::AddRadio(this->parameters.grid.layout, labels, 2))
					{
						this->clearGrid();
					}
					if (ofxPreset::Gui::AddParameter(this->parameters.grid.size))
					{
						this->clearGrid();
					}
					ofxPreset::Gui::AddParameter(this->parameters.grid.centerPoints);
					ofxPreset::Gui::AddParameter(this->parameters.grid.horizontalLines);
					ofxPreset::Gui::AddParameter(this->parameters.grid.verticalLines);
					ofxPreset::Gui::AddParameter(this->parameters.grid.crossLines);
				}

				// Add parameters by group.
				ofxPreset::Gui::AddGroup(this->parameters.border, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		/// Do something after the parameters are saved.
		/// You can save other stuff to the same json object here too.
		void Calibrate::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		/// Do something after the parameters are loaded.
		/// You can load your other stuff here from that json object.
		/// You can also set any refresh flags if necessary.
		void Calibrate::deserialize(const nlohmann::json & json)
		{

		}
	}
}