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
		{
			this->parameterListeners.push_back(this->parameters.grid.layout.newListener([this](int &)
			{
				this->clearGrid();
			}));
			this->parameterListeners.push_back(this->parameters.grid.resolution.newListener([this](int &)
			{
				this->clearGrid();
			}));
			this->parameterListeners.push_back(this->parameters.grid.lineWidth.newListener([this](float &)
			{
				this->clearGrid();
			}));
		}

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
			const auto useLines = (this->parameters.grid.lineWidth == 1.0f);

			if (this->parameters.grid.centerPoints && this->pointsMesh.getNumVertices() == 0)
			{
				// Rebuild points.
				const auto numPointsHorz = GetCanvasWidth(layout) / this->parameters.grid.resolution;
				const auto numPointsVert = GetCanvasHeight(layout) / this->parameters.grid.resolution;
				const auto color = ofFloatColor::cyan;
				this->pointsMesh.setMode(useLines ? OF_PRIMITIVE_POINTS : OF_PRIMITIVE_TRIANGLES);
				for (int j = 0; j < numPointsVert; ++j)
				{
					for (int i = 0; i < numPointsHorz; ++i)
					{
						auto centerPoint = glm::vec3(this->parameters.grid.resolution / 2 + i * this->parameters.grid.resolution, this->parameters.grid.resolution / 2 + j * this->parameters.grid.resolution, 0);
						if (useLines)
						{
							this->pointsMesh.addVertex(centerPoint);
							this->pointsMesh.addColor(color);
						}
						else
						{
							this->addQuad(centerPoint, glm::vec3(this->parameters.grid.lineWidth), color, this->pointsMesh);
						}
					}
				}
			}

			if (this->parameters.grid.horizontalLines && this->horizontalMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto numLines = GetCanvasHeight(layout) / this->parameters.grid.resolution + 1;
				this->horizontalMesh.setMode(useLines ? OF_PRIMITIVE_LINES : OF_PRIMITIVE_TRIANGLES);
				for (int i = 0; i < numLines; ++i)
				{
					const auto color = (i % 4 == 0) ? ofFloatColor::red : ofFloatColor::yellow;
					if (useLines)
					{
						this->horizontalMesh.addVertex(glm::vec3(0, i * this->parameters.grid.resolution, 0));
						this->horizontalMesh.addVertex(glm::vec3(GetCanvasWidth(layout), i * this->parameters.grid.resolution, 0));
						this->horizontalMesh.addColor(color);
						this->horizontalMesh.addColor(color);
					}
					else
					{
						this->addQuad(glm::vec3(GetCanvasWidth(layout) * 0.5f, i * this->parameters.grid.resolution, 0), glm::vec3(GetCanvasWidth(layout), this->parameters.grid.lineWidth, 0.0f), color, this->horizontalMesh);
					}
				}
			}

			if (this->parameters.grid.verticalLines && this->verticalMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto numLines = GetCanvasWidth(layout) / this->parameters.grid.resolution + 1;
				this->verticalMesh.setMode(useLines ? OF_PRIMITIVE_LINES : OF_PRIMITIVE_TRIANGLES);
				for (int i = 0; i < numLines; ++i)
				{
					const auto color = (i % 4 == 0) ? ofFloatColor::green : ofFloatColor::yellow;
					if (useLines)
					{
						this->verticalMesh.addVertex(glm::vec3(i * this->parameters.grid.resolution, 0, 0));
						this->verticalMesh.addVertex(glm::vec3(i * this->parameters.grid.resolution, GetCanvasHeight(layout), 0));
						this->verticalMesh.addColor(color);
						this->verticalMesh.addColor(color);
					}
					else
					{
						this->addQuad(glm::vec3(i * this->parameters.grid.resolution, GetCanvasHeight(layout) * 0.5f, 0), glm::vec3(this->parameters.grid.lineWidth, GetCanvasHeight(layout), 0.0f), color, this->horizontalMesh);
					}
				}
			}

			if (this->parameters.grid.crossLines && this->crossMesh.getNumVertices() == 0)
			{
				// Rebuild lines.
				const auto color = ofFloatColor::magenta;
				this->crossMesh.setMode(useLines ? OF_PRIMITIVE_LINES : OF_PRIMITIVE_TRIANGLES);
				if (useLines)
				{
					this->crossMesh.addVertex(glm::vec3(0, 0, 0));
					this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), GetCanvasHeight(layout), 0));
					this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), 0, 0));
					this->crossMesh.addVertex(glm::vec3(0, GetCanvasHeight(layout), 0));
					for (int i = 0; i < 4; ++i)
					{
						this->crossMesh.addColor(color);
					}
				}
				else
				{
					const auto offset = sqrtf((this->parameters.grid.lineWidth * this->parameters.grid.lineWidth) / 2.0f);

					// Top-Left to Bottom-Right.
					{
						const auto startIdx = this->crossMesh.getNumVertices();

						this->crossMesh.addVertex(glm::vec3(0, 0 + offset, 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout) - offset, GetCanvasHeight(layout), 0));
						this->crossMesh.addVertex(glm::vec3(0, 0, 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), GetCanvasHeight(layout), 0));
						this->crossMesh.addVertex(glm::vec3(0 + offset, 0, 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), GetCanvasHeight(layout) - offset, 0));

						for (int i = 0; i < 6; ++i)
						{
							this->crossMesh.addColor(color);
						}

						this->crossMesh.addIndex(startIdx + 0);
						this->crossMesh.addIndex(startIdx + 1);
						this->crossMesh.addIndex(startIdx + 2);

						this->crossMesh.addIndex(startIdx + 1);
						this->crossMesh.addIndex(startIdx + 2);
						this->crossMesh.addIndex(startIdx + 3);

						this->crossMesh.addIndex(startIdx + 2);
						this->crossMesh.addIndex(startIdx + 3);
						this->crossMesh.addIndex(startIdx + 4);

						this->crossMesh.addIndex(startIdx + 3);
						this->crossMesh.addIndex(startIdx + 4);
						this->crossMesh.addIndex(startIdx + 5);
					}

					// Bottom-Left to Top-Right.
					{
						const auto startIdx = this->crossMesh.getNumVertices();

						this->crossMesh.addVertex(glm::vec3(0, GetCanvasHeight(layout) - offset, 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout) - offset, 0, 0));
						this->crossMesh.addVertex(glm::vec3(0, GetCanvasHeight(layout), 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), 0, 0));
						this->crossMesh.addVertex(glm::vec3(0 + offset, GetCanvasHeight(layout), 0));
						this->crossMesh.addVertex(glm::vec3(GetCanvasWidth(layout), 0 + offset, 0));

						for (int i = 0; i < 6; ++i)
						{
							this->crossMesh.addColor(color);
						}

						this->crossMesh.addIndex(startIdx + 0);
						this->crossMesh.addIndex(startIdx + 1);
						this->crossMesh.addIndex(startIdx + 2);

						this->crossMesh.addIndex(startIdx + 1);
						this->crossMesh.addIndex(startIdx + 2);
						this->crossMesh.addIndex(startIdx + 3);

						this->crossMesh.addIndex(startIdx + 2);
						this->crossMesh.addIndex(startIdx + 3);
						this->crossMesh.addIndex(startIdx + 4);

						this->crossMesh.addIndex(startIdx + 3);
						this->crossMesh.addIndex(startIdx + 4);
						this->crossMesh.addIndex(startIdx + 5);
					}
				}
			}
		}

		//--------------------------------------------------------------
		void Calibrate::addQuad(const glm::vec3 & center, const glm::vec3 & dimensions, const ofFloatColor & color, ofVboMesh & mesh)
		{
			const auto startIdx = mesh.getNumVertices();
			const auto halfDims = dimensions * 0.5f;

			mesh.addVertex(glm::vec3(center.x - halfDims.x, center.y + halfDims.y, 0.0f));
			mesh.addVertex(glm::vec3(center.x - halfDims.x, center.y - halfDims.y, 0.0f));
			mesh.addVertex(glm::vec3(center.x + halfDims.x, center.y + halfDims.y, 0.0f));
			mesh.addVertex(glm::vec3(center.x + halfDims.x, center.y - halfDims.y, 0.0f));

			for (int i = 0; i < 4; ++i)
			{
				mesh.addColor(color);
			}

			mesh.addIndex(startIdx + 0);
			mesh.addIndex(startIdx + 1);
			mesh.addIndex(startIdx + 2);

			mesh.addIndex(startIdx + 1);
			mesh.addIndex(startIdx + 2);
			mesh.addIndex(startIdx + 3);
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
				if (ofxPreset::Gui::BeginTree(this->parameters.grid, settings))
				{
					static vector<string> labels{ "Back", "Front" };
					ofxPreset::Gui::AddRadio(this->parameters.grid.layout, labels, 2);
					ofxPreset::Gui::AddParameter(this->parameters.grid.resolution);
					ofxPreset::Gui::AddParameter(this->parameters.grid.lineWidth);
					ofxPreset::Gui::AddParameter(this->parameters.grid.centerPoints);
					ofxPreset::Gui::AddParameter(this->parameters.grid.horizontalLines);
					ofxPreset::Gui::AddParameter(this->parameters.grid.verticalLines);
					ofxPreset::Gui::AddParameter(this->parameters.grid.crossLines);

					ofxPreset::Gui::EndTree(settings);
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