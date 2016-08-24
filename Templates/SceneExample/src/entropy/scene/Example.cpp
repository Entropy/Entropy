#include "Example.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Example::Example()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Example::~Example()
		{}

		//--------------------------------------------------------------
		// Set up your crap here!
		void Example::setup()
		{
			ofEnableLighting();
		}
		
		//--------------------------------------------------------------
		// Clean up your crap here!
		void Example::exit()
		{
			
		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void Example::resizeBack(ofResizeEventArgs & args)
		{
			if (this->getGridLayout() == render::Layout::Back)
			{
				this->clearGrid();
			}
		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void Example::resizeFront(ofResizeEventArgs & args)
		{
			if (this->getGridLayout() == render::Layout::Front)
			{
				this->clearGrid();
			}
		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void Example::update(double dt)
		{
			if (this->box.getSize().x != this->parameters.box.size)
			{
				this->box.set(this->parameters.box.size);
				light.setPosition(this->box.getSize() * 2);
			}

			this->updateGrid(this->getGridLayout());
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Example::drawBackBase()
		{

		}
		
		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Example::drawBackWorld()
		{
			auto mode = static_cast<DrawMode>(this->parameters.box.drawModeBack.get());
			this->drawScene(mode);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawBackOverlay()
		{
			if (this->parameters.border.drawBack)
			{
				this->drawBorder(render::Layout::Back);
			}

			if (this->getGridLayout() == render::Layout::Back)
			{
				this->drawGrid();
			}
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Example::drawFrontBase()
		{

		}

		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Example::drawFrontWorld()
		{
			auto mode = static_cast<DrawMode>(this->parameters.box.drawModeFront.get());
			this->drawScene(mode);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawFrontOverlay()
		{
			if (this->parameters.border.drawFront)
			{
				this->drawBorder(render::Layout::Front);
			}

			if (this->getGridLayout() == render::Layout::Front)
			{
				this->drawGrid();
			}
		}

		//--------------------------------------------------------------
		void Example::drawScene(DrawMode drawMode)
		{
			if (drawMode == DrawMode::None) return;

			ofPushStyle();
			{
				ofEnableDepthTest();
				material.setDiffuseColor(this->parameters.box.color);
				light.enable();
				glEnable(GL_CULL_FACE);
				ofSetColor(this->parameters.box.color.get());
				material.begin();
				this->box.draw((drawMode == DrawMode::Fill) ? OF_MESH_FILL : OF_MESH_WIREFRAME);
				glDisable(GL_CULL_FACE);
				material.end();
				light.disable();
				ofDisableDepthTest();
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		void Example::drawBorder(render::Layout layout)
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
		render::Layout Example::getGridLayout()
		{
			return static_cast<render::Layout>(this->parameters.grid.layout.get());
		}

		//--------------------------------------------------------------
		void Example::clearGrid()
		{
			this->pointsMesh.clear();
			this->horizontalMesh.clear();
			this->verticalMesh.clear();
			this->crossMesh.clear();
		}

		//--------------------------------------------------------------
		void Example::updateGrid(render::Layout layout)
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
		void Example::drawGrid()
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
		// Add Scene specific GUI windows here.
		void Example::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				// Add parameters manually.
				if (ImGui::CollapsingHeader(this->parameters.box.getName().c_str(), nullptr, true, true))
				{
					static vector<string> labels{ "None", "Wire", "Fill" };
					ofxPreset::Gui::AddRadio(this->parameters.box.drawModeBack, labels, 3);
					ofxPreset::Gui::AddRadio(this->parameters.box.drawModeFront, labels, 3);
					ofxPreset::Gui::AddParameter(this->parameters.box.size);
					ofxPreset::Gui::AddParameter(this->parameters.box.color);
				}

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
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void Example::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void Example::deserialize(const nlohmann::json & json)
		{

		}
	}
}