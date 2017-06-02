#include "App.h"

#include "ofxPreset.h"

#include "entropy/render/Layout.h"
#include "entropy/Helpers.h"

namespace entropy
{
	namespace util
	{
		//--------------------------------------------------------------
		App_::App_()
		{
			// Instantiate attributes.
			this->canvas[render::Layout::Back] = make_shared<render::Canvas>(render::Layout::Back);
			this->canvas[render::Layout::Front] = make_shared<render::Canvas>(render::Layout::Front);
			this->messenger = make_shared<util::Messenger>();
			this->playlist = make_shared<scene::Playlist>();

#ifdef OFX_PARAMETER_TWISTER
			this->twister = make_shared<pal::Kontrol::ofxParameterTwister>();
			this->twister->setup();
#endif

			// Setup gui.
			this->imGui.setup();
			this->controlsVisible = true;

			// Register events listeners.
			ofAddListener(ofEvents().update, this, &App_::onUpdate);
			ofAddListener(ofEvents().draw, this, &App_::onDraw);

			ofAddListener(ofEvents().keyPressed, this, &App_::onKeyPressed);
			ofAddListener(ofEvents().keyReleased, this, &App_::onKeyReleased);

			ofAddListener(ofEvents().mouseMoved, this, &App_::onMouseMoved);
			ofAddListener(ofEvents().mousePressed, this, &App_::onMousePressed);
			ofAddListener(ofEvents().mouseDragged, this, &App_::onMouseDragged);
			ofAddListener(ofEvents().mouseReleased, this, &App_::onMouseReleased);

			ofAddListener(this->canvas[render::Layout::Back]->resizeEvent, this, &App_::onCanvasBackResized);
			ofAddListener(this->canvas[render::Layout::Front]->resizeEvent, this, &App_::onCanvasFrontResized);
			//ofAddListener(ofEvents().windowResized, this, &App_::onWindowResized);

			// Set base parameter listeners.
			this->parameterListeners.push_back(parameters.controlScreen.preview.backWarp.newListener([this](bool &)
			{
				this->updatePreviews();
			}));
			this->parameterListeners.push_back(parameters.controlScreen.preview.backCanvas.newListener([this](bool &)
			{
				this->updatePreviews();
			}));
			this->parameterListeners.push_back(parameters.controlScreen.preview.frontWarp.newListener([this](bool &)
			{
				this->updatePreviews();
			}));
			this->parameterListeners.push_back(parameters.controlScreen.preview.frontCanvas.newListener([this](bool &)
			{
				this->updatePreviews();
			}));
			this->parameterListeners.push_back(parameters.controlScreen.preview.scale.newListener([this](float &)
			{
				this->updatePreviews();
			}));

			// Load initial settings, if any.
			this->loadSettings();

			// Apply initial configuration.
			this->applyConfiguration();
		}

		//--------------------------------------------------------------
		App_::~App_()
		{
			// Unregister event listeners.
			ofRemoveListener(ofEvents().update, this, &App_::onUpdate);
			ofRemoveListener(ofEvents().draw, this, &App_::onDraw);

			ofRemoveListener(ofEvents().keyPressed, this, &App_::onKeyPressed);
			ofRemoveListener(ofEvents().keyReleased, this, &App_::onKeyReleased);

			ofRemoveListener(ofEvents().mouseMoved, this, &App_::onMouseMoved);
			ofRemoveListener(ofEvents().mousePressed, this, &App_::onMousePressed);
			ofRemoveListener(ofEvents().mouseDragged, this, &App_::onMouseDragged);
			ofRemoveListener(ofEvents().mouseReleased, this, &App_::onMouseReleased);

			ofRemoveListener(this->canvas[render::Layout::Back]->resizeEvent, this, &App_::onCanvasBackResized);
			ofRemoveListener(this->canvas[render::Layout::Front]->resizeEvent, this, &App_::onCanvasFrontResized);
			//ofRemoveListener(ofEvents().windowResized, this, &App_::onWindowResized);

			this->imGui.close();

			// Reset pointers.
			this->playlist.reset();
			this->messenger.reset();
			for (auto & it : this->canvas)
			{
				it.second.reset();
			}
			this->canvas.clear();
		}

		//--------------------------------------------------------------
		const std::filesystem::path & App_::getDataPath()
		{
			static std::filesystem::path dataPath;
			if (dataPath.empty())
			{
				dataPath = GetSharedDataPath() / "entropy" / "util" / "App";
			}
			return dataPath;
		}

		//--------------------------------------------------------------
		const std::filesystem::path & App_::getSettingsFilePath()
		{
			static std::filesystem::path filePath;
			if (filePath.empty())
			{
				filePath = this->getDataPath() / "settings.json";
			}
			return filePath;
		}

		//--------------------------------------------------------------
		bool App_::loadSettings()
		{
			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::ReadOnly);
			if (!file.exists())
			{
				ofLogWarning(__FUNCTION__) << "File not found at path " << filePath;
				return false;
			}

			nlohmann::json json;
			file >> json;

			ofxPreset::Serializer::Deserialize(json, this->parameters);
			this->messenger->deserialize(json);

			return true;
		}

		//--------------------------------------------------------------
		bool App_::saveSettings()
		{
			nlohmann::json json;
			ofxPreset::Serializer::Serialize(json, this->parameters);
			this->messenger->serialize(json);

			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::WriteOnly);
			file << json.dump(4);

			return true;
		}

		//--------------------------------------------------------------
		std::shared_ptr<render::Canvas> App_::getCanvas(render::Layout layout)
		{
			return this->canvas[layout];
		}

		//--------------------------------------------------------------
		std::shared_ptr<util::Messenger> App_::getMessenger() const
		{
			return this->messenger;
		}

		//--------------------------------------------------------------
		std::shared_ptr<scene::Playlist> App_::getPlaylist() const
		{
			return this->playlist;
		}

#ifdef OFX_PARAMETER_TWISTER
		//--------------------------------------------------------------
		std::shared_ptr<pal::Kontrol::ofxParameterTwister> App_::getTwister() const
		{
			return this->twister;
		}
#endif

		//--------------------------------------------------------------
		const ofRectangle & App_::getScreenBounds(render::Layout layout)
		{
			return this->screenBounds[layout];
		}

		//--------------------------------------------------------------
		const ofRectangle & App_::getBoundsControl() const
		{
			return this->boundsControl;
		}

		//--------------------------------------------------------------
		bool App_::isMouseOverGui() const
		{
			return this->guiSettings.mouseOverGui;
		}

		//--------------------------------------------------------------
		bool App_::isControlsVisible() const
		{
			return this->controlsVisible;
		}

		//--------------------------------------------------------------
		void App_::onUpdate(ofEventArgs & args)
		{
			if (this->controlsVisible || ofGetWindowMode() == OF_WINDOW)
			{
				ofShowCursor();
			}
			else
			{
				ofHideCursor();
			}

			for (auto & it : this->canvas)
			{
				it.second->update();
			}

			this->messenger->update();

#ifdef OFX_PARAMETER_TWISTER
			this->twister->update();
#endif

			auto dt = ofGetLastFrameTime();
			this->playlist->update(dt);
		}
		
		//--------------------------------------------------------------
		void App_::onDraw(ofEventArgs & args)
		{
			ofBackground(this->parameters.background.get());

			auto scene = GetCurrentScene();
			if (scene)
			{
				ofSetColor(ofColor::white);

				// Back screen.
				if (this->parameters.backScreen.enabled || this->parameters.controlScreen.enabled)
				{
					this->processCanvas(render::Layout::Back, this->parameters.backScreen.enabled);
				}

				// Front screen.
				if (this->parameters.frontScreen.enabled || this->parameters.controlScreen.enabled)
				{
					this->processCanvas(render::Layout::Front, this->parameters.frontScreen.enabled);
				}

				// Control screen.
				if (this->parameters.controlScreen.enabled)
				{
					if (this->parameters.controlScreen.preview.backWarp)
					{
						this->canvas[render::Layout::Back]->render(this->previewData[render::Layout::Back].warpBounds);
						this->previewData[render::Layout::Back].warpOutlines.draw();
					}
					if (this->parameters.controlScreen.preview.backCanvas)
					{
						this->canvas[render::Layout::Back]->getRenderTexture().draw(this->previewData[render::Layout::Back].canvasBounds);
					}

					if (this->parameters.controlScreen.preview.frontWarp)
					{
						this->canvas[render::Layout::Front]->render(this->previewData[render::Layout::Front].warpBounds);
						this->previewData[render::Layout::Front].warpOutlines.draw();
					}
					if (this->parameters.controlScreen.preview.frontCanvas)
					{
						this->canvas[render::Layout::Front]->getRenderTexture().draw(this->previewData[render::Layout::Front].canvasBounds);
					}
				}
			}

			if (!this->controlsVisible) return;

			this->guiSettings.mouseOverGui = this->canvas[render::Layout::Back]->isEditing() || this->canvas[render::Layout::Front]->isEditing();
			this->guiSettings.windowPos = ofVec2f(kImGuiMargin, kImGuiMargin);
			this->guiSettings.windowSize = ofVec2f::zero();
			if (this->parameters.controlScreen.enabled)
			{
				this->guiSettings.screenBounds = this->boundsControl;
			}
			else if (this->parameters.backScreen.enabled)
			{
				this->guiSettings.screenBounds = this->screenBounds[render::Layout::Back];
			}
			else if (this->parameters.frontScreen.enabled)
			{
				this->guiSettings.screenBounds = this->screenBounds[render::Layout::Front];
			}
			else
			{
				this->guiSettings.screenBounds = ofRectangle(0.0f, 0.0f, ofGetWidth(), ofGetHeight());
			}

			// Draw the timeline overlay.
			this->playlist->drawTimeline(this->guiSettings);

			// Draw the gui overlay.
			this->imGui.begin();
			{
				this->drawGui(this->guiSettings);

				for (auto & it : this->canvas)
				{
					it.second->drawGui(this->guiSettings);
				}

				this->messenger->drawGui(this->guiSettings);

				this->playlist->drawGui(this->guiSettings);
			}
			this->imGui.end();
		}

		//--------------------------------------------------------------
		void App_::processCanvas(render::Layout layout, bool renderEnabled)
		{
			auto scene = GetCurrentScene();
			if (scene)
			{
				// Draw the base and world content.
				this->canvas[layout]->beginDraw();
				{
					this->playlist->drawSceneBase(layout);
					this->playlist->drawSceneWorld(layout);
					this->playlist->drawSceneOverlay(layout);
				}
				this->canvas[layout]->endDraw();

				if (renderEnabled)
				{
					// Render the scene.
					this->canvas[layout]->render(this->screenBounds[layout]);
				}
			}
		}

		//--------------------------------------------------------------
		void App_::drawGui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginWindow(this->parameters.getName(), this->guiSettings))
			{
				ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ofGetFrameRate());

				if (ImGui::Button("Save"))
				{
					this->applyConfiguration();
					this->saveSettings();
				}
				ImGui::SameLine();
				if (ImGui::Button("Load"))
				{
					this->loadSettings();
				}

				ofxImGui::AddParameter(this->parameters.background);

				if (ofxImGui::BeginTree(this->parameters.controlScreen, settings))
				{
					if (ofxImGui::AddParameter(this->parameters.controlScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxImGui::AddStepper(this->parameters.controlScreen.screenWidth);
					ofxImGui::AddStepper(this->parameters.controlScreen.screenHeight);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					if (ofxImGui::BeginTree(this->parameters.controlScreen.preview, settings))
					{
						ImGui::Text("Back Screen %d x %d", static_cast<int>(this->screenBounds[render::Layout::Back].getWidth()), static_cast<int>(this->screenBounds[render::Layout::Back].getHeight()));
						ImGui::Text("Back Canvas %d x %d", static_cast<int>(this->canvas[render::Layout::Back]->getWidth()), static_cast<int>(this->canvas[render::Layout::Back]->getHeight()));

						ofxImGui::AddParameter(parameters.controlScreen.preview.backWarp);
						ImGui::SameLine();
						ofxImGui::AddParameter(parameters.controlScreen.preview.backCanvas);
						
						ImGui::Text("Front Screen %d x %d", static_cast<int>(this->screenBounds[render::Layout::Front].getWidth()), static_cast<int>(this->screenBounds[render::Layout::Front].getHeight()));
						ImGui::Text("Front Canvas %d x %d", static_cast<int>(this->canvas[render::Layout::Front]->getWidth()), static_cast<int>(this->canvas[render::Layout::Front]->getHeight()));
						
						ofxImGui::AddParameter(parameters.controlScreen.preview.frontWarp);
						ImGui::SameLine();
						ofxImGui::AddParameter(parameters.controlScreen.preview.frontCanvas);
						
						ofxImGui::AddParameter(this->parameters.controlScreen.preview.scale);

						ofxImGui::EndTree(settings);
					}

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.backScreen, settings))
				{
					if (ofxImGui::AddParameter(this->parameters.backScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxImGui::AddStepper(this->parameters.backScreen.screenWidth);
					ofxImGui::AddStepper(this->parameters.backScreen.screenHeight);
					ofxImGui::AddStepper(this->parameters.backScreen.numRows);
					ofxImGui::AddStepper(this->parameters.backScreen.numCols);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.frontScreen, settings))
				{
					if (ofxImGui::AddParameter(this->parameters.frontScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxImGui::AddStepper(this->parameters.frontScreen.screenWidth);
					ofxImGui::AddStepper(this->parameters.frontScreen.screenHeight);
					ofxImGui::AddStepper(this->parameters.frontScreen.numRows);
					ofxImGui::AddStepper(this->parameters.frontScreen.numCols);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					ofxImGui::EndTree(settings);
				}
			}
			ofxImGui::EndWindow(this->guiSettings);
		}

		//--------------------------------------------------------------
		void App_::applyConfiguration()
		{
			// Calculate the bounds per screen.
			ofRectangle totalBounds;

			this->boundsControl = ofRectangle(totalBounds.getMaxX(), totalBounds.getMinY(), this->parameters.controlScreen.screenWidth, this->parameters.controlScreen.screenHeight);
			if (this->parameters.controlScreen.enabled)
			{
				totalBounds.growToInclude(this->boundsControl);
			}

			this->screenBounds[render::Layout::Back] = ofRectangle(totalBounds.getMaxX(), totalBounds.getMinY(), this->parameters.backScreen.screenWidth * this->parameters.backScreen.numCols, this->parameters.backScreen.screenHeight * this->parameters.backScreen.numRows);
			if (this->parameters.backScreen.enabled)
			{
				totalBounds.growToInclude(this->screenBounds[render::Layout::Back]);
			}

			this->screenBounds[render::Layout::Front] = ofRectangle(totalBounds.getMaxX(), totalBounds.getMinY(), this->parameters.frontScreen.screenWidth * this->parameters.frontScreen.numCols, this->parameters.frontScreen.screenHeight * this->parameters.frontScreen.numRows);
			if (this->parameters.frontScreen.enabled)
			{
				totalBounds.growToInclude(this->screenBounds[render::Layout::Front]);
			}

			// Resize the window.
			ofSetWindowShape(totalBounds.width, totalBounds.height);

			// Force call the callback, in case the actual application window doesn't resize.
			// This happens if there aren't enough screens connected to span the whole thing.
			auto resizeArgs = ofResizeEventArgs(totalBounds.width, totalBounds.height);
			this->onWindowResized(resizeArgs);
		}

		//--------------------------------------------------------------
		void App_::updatePreviews()
		{
			if (this->parameters.controlScreen.enabled)
			{
				float currY = 0.0f;
				ofRectangle tmpBounds;

				// Back Warp.
				tmpBounds.height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				tmpBounds.width = this->screenBounds[render::Layout::Back].getWidth() * tmpBounds.height / this->screenBounds[render::Layout::Back].getHeight();
				tmpBounds.x = (this->boundsControl.getWidth() - tmpBounds.getWidth()) * 0.5f;
				tmpBounds.y = this->boundsControl.getMinY() + kImGuiMargin + currY;
				this->previewData[render::Layout::Back].warpBounds = tmpBounds;

				this->updateOutline(render::Layout::Back);

				if (this->parameters.controlScreen.preview.backWarp)
				{
					// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
					this->playlist->setCameraControlArea(render::Layout::Back, this->previewData[render::Layout::Back].warpBounds);
					
					// Update next preview position.
					currY = this->previewData[render::Layout::Back].warpBounds.getMaxY();
				}

				// Back Canvas.
				tmpBounds.height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				tmpBounds.width = this->canvas[render::Layout::Back]->getWidth() * tmpBounds.height / this->canvas[render::Layout::Back]->getHeight();
				tmpBounds.x = (this->boundsControl.getWidth() - tmpBounds.getWidth()) * 0.5f;
				tmpBounds.y = this->boundsControl.getMinY() + kImGuiMargin + currY;
				this->previewData[render::Layout::Back].canvasBounds = tmpBounds;

				if (this->parameters.controlScreen.preview.backCanvas)
				{
					// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
					this->playlist->setCameraControlArea(render::Layout::Back, this->previewData[render::Layout::Back].canvasBounds);

					// Update next preview position.
					currY = this->previewData[render::Layout::Back].canvasBounds.getMaxY();
				}

				// Front Warp.
				tmpBounds.height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				tmpBounds.width = this->screenBounds[render::Layout::Front].getWidth() * tmpBounds.height / this->screenBounds[render::Layout::Front].getHeight();
				tmpBounds.x = (this->boundsControl.getWidth() - tmpBounds.getWidth()) * 0.5f;
				tmpBounds.y = this->boundsControl.getMinY() + kImGuiMargin + currY;
				this->previewData[render::Layout::Front].warpBounds = tmpBounds;

				this->updateOutline(render::Layout::Front);

				if (this->parameters.controlScreen.preview.frontWarp)
				{
					// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
					this->playlist->setCameraControlArea(render::Layout::Front, this->previewData[render::Layout::Front].warpBounds);

					// Update next preview position.
					currY = this->previewData[render::Layout::Front].warpBounds.getMaxY();
				}

				// Front Canvas.
				tmpBounds.height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				tmpBounds.width = this->canvas[render::Layout::Front]->getWidth() * tmpBounds.height / this->canvas[render::Layout::Front]->getHeight();
				tmpBounds.x = (this->boundsControl.getWidth() - tmpBounds.getWidth()) * 0.5f;
				tmpBounds.y = this->boundsControl.getMinY() + kImGuiMargin + currY;
				this->previewData[render::Layout::Front].canvasBounds = tmpBounds;

				if (this->parameters.controlScreen.preview.frontCanvas)
				{
					// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
					this->playlist->setCameraControlArea(render::Layout::Front, this->previewData[render::Layout::Front].canvasBounds);

					// Update next preview position.
					currY = this->previewData[render::Layout::Front].canvasBounds.getMaxY();
				}
			}
			else
			{
				// Set the Scene cameras to use the Canvas bounds as mouse-enabled areas.
				this->playlist->setCameraControlArea(render::Layout::Back, this->screenBounds[render::Layout::Back]);
				this->playlist->setCameraControlArea(render::Layout::Front, this->screenBounds[render::Layout::Front]);
			}
		}

		//--------------------------------------------------------------
		void App_::updateOutline(render::Layout layout)
		{
			const auto & bounds = this->previewData[layout].warpBounds;

			ofVboMesh tmpOutline;
			tmpOutline.setMode(OF_PRIMITIVE_LINES);

			// Outline.
			tmpOutline.addVertex(glm::vec3(bounds.getMinX(), bounds.getMinY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMaxX(), bounds.getMinY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMaxX(), bounds.getMinY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMaxX(), bounds.getMaxY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMaxX(), bounds.getMaxY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMinX(), bounds.getMaxY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMinX(), bounds.getMaxY(), 0));
			tmpOutline.addVertex(glm::vec3(bounds.getMinX(), bounds.getMinY(), 0));

			// Columns.
			int numCols = (layout == render::Layout::Back) ? this->parameters.backScreen.numCols : this->parameters.frontScreen.numCols;
			if (numCols > 1)
			{
				float colWidth = bounds.getWidth() / numCols;
				float currX = bounds.getMinX();
				for (int i = 0; i < numCols; ++i)
				{
					tmpOutline.addVertex(glm::vec3(currX, bounds.getMinY(), 0));
					tmpOutline.addVertex(glm::vec3(currX, bounds.getMaxY(), 0));
					currX += colWidth;
				}
			}

			// Rows.
			int numRows = (layout == render::Layout::Back) ? this->parameters.backScreen.numRows : this->parameters.frontScreen.numRows;
			if (numRows > 1)
			{
				float rowHeight = bounds.getHeight() / numRows;
				float currY = bounds.getMinY();
				for (int i = 0; i < numRows; ++i)
				{
					tmpOutline.addVertex(glm::vec3(bounds.getMinX(), currY, 0));
					tmpOutline.addVertex(glm::vec3(bounds.getMaxX(), currY, 0));
					currY += rowHeight;
				}
			}

			this->previewData[layout].warpOutlines = tmpOutline;
		}
		
		//--------------------------------------------------------------
		void App_::onKeyPressed(ofKeyEventArgs & args)
		{
			if (args.keycode == GLFW_KEY_F && (ofGetKeyPressed(OF_KEY_CONTROL) || ofGetKeyPressed(OF_KEY_COMMAND)))
			{
				ofToggleFullscreen();
				return;
			}
			if (args.key == 'g')
			{
				this->controlsVisible ^= 1;
			}
			if (this->canvas[render::Layout::Back]->keyPressed(args))
			{
				return;
			}
			if (this->canvas[render::Layout::Front]->keyPressed(args))
			{
				return;
			}
			if (this->playlist->keyPressed(args))
			{
				return;
			}
		}

		//--------------------------------------------------------------
		void App_::onKeyReleased(ofKeyEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void App_::onMouseMoved(ofMouseEventArgs & args)
		{
			for (auto & it : this->screenBounds)
			{
				if (it.second.inside(args))
				{
					const auto screenPos = glm::vec2(args.x - it.second.getMinX(), args.y - it.second.getMinY());
					this->canvas[it.first]->cursorMoved(screenPos);
				}
			}
		}

		//--------------------------------------------------------------
		void App_::onMousePressed(ofMouseEventArgs & args)
		{
			for (auto & it : this->screenBounds)
			{
				if (it.second.inside(args))
				{
					const auto screenPos = glm::vec2(args.x - it.second.getMinX(), args.y - it.second.getMinY());
					this->canvas[it.first]->cursorDown(screenPos);
				}
			}
		}

		//--------------------------------------------------------------
		void App_::onMouseDragged(ofMouseEventArgs & args)
		{
			for (auto & it : this->screenBounds)
			{
				if (it.second.inside(args))
				{
					const auto screenPos = glm::vec2(args.x - it.second.getMinX(), args.y - it.second.getMinY());
					this->canvas[it.first]->cursorDragged(screenPos);
				}
			}
		}

		//--------------------------------------------------------------
		void App_::onMouseReleased(ofMouseEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void App_::onCanvasBackResized(ofResizeEventArgs & args)
		{
			this->updatePreviews();

			this->playlist->canvasResized(render::Layout::Back, args);
		}

		//--------------------------------------------------------------
		void App_::onCanvasFrontResized(ofResizeEventArgs & args)
		{
			this->updatePreviews();

			this->playlist->canvasResized(render::Layout::Front, args);
		}

		//--------------------------------------------------------------
		void App_::onWindowResized(ofResizeEventArgs & args)
		{
			this->updatePreviews();

			// Notify listeners.
			auto resizeBackArgs = ofResizeEventArgs(this->screenBounds[render::Layout::Back].width, this->screenBounds[render::Layout::Back].height);
			this->canvas[render::Layout::Back]->screenResized(resizeBackArgs);

			auto resizeFrontArgs = ofResizeEventArgs(this->screenBounds[render::Layout::Front].width, this->screenBounds[render::Layout::Front].height);
			this->canvas[render::Layout::Front]->screenResized(resizeFrontArgs);
		}
	}
}
