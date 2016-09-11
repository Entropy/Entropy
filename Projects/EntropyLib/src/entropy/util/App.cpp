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
			this->playlist = make_shared<scene::Playlist>();

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
			this->parameterListeners.push_back(parameters.controlScreen.preview.scale.newListener([this](float & value)
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
			for (auto & it : this->canvas)
			{
				it.second.reset();
			}
			this->canvas.clear();
		}

		//--------------------------------------------------------------
		const string & App_::getDataPath()
		{
			static string dataPath;
			if (dataPath.empty())
			{
				dataPath = GetSharedDataPath();
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("entropy"));
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("util"));
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("App"));
			}
			return dataPath;
		}

		//--------------------------------------------------------------
		const string & App_::getSettingsFilePath()
		{
			static string filePath;
			if (filePath.empty())
			{
				filePath = this->getDataPath();
				filePath.append("settings.json");
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

			return true;
		}

		//--------------------------------------------------------------
		bool App_::saveSettings()
		{
			nlohmann::json json;
			ofxPreset::Serializer::Serialize(json, this->parameters);

			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::WriteOnly);
			file << json.dump(4);

			return true;
		}

		//--------------------------------------------------------------
		shared_ptr<render::Canvas> App_::getCanvas(render::Layout layout)
		{
			return this->canvas[layout];
		}

		//--------------------------------------------------------------
		shared_ptr<scene::Playlist> App_::getPlaylist() const
		{
			return this->playlist;
		}

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
				// Back screen.
				if (this->parameters.backScreen.enabled || (this->parameters.controlScreen.enabled && this->parameters.controlScreen.preview.backEnabled))
				{
					this->processCanvas(render::Layout::Back, this->parameters.backScreen.enabled);
				}

				// Front screen.
				if (this->parameters.frontScreen.enabled || (this->parameters.controlScreen.enabled && this->parameters.controlScreen.preview.frontEnabled))
				{
					this->processCanvas(render::Layout::Front, this->parameters.frontScreen.enabled);
				}

				// Control screen.
				if (this->parameters.controlScreen.enabled)
				{
					if (this->parameters.controlScreen.preview.backEnabled)
					{
						this->canvas[render::Layout::Back]->getRenderTexture().draw(this->previewBounds[render::Layout::Back]);
					}
					if (this->parameters.controlScreen.preview.frontEnabled)
					{
						this->canvas[render::Layout::Front]->getRenderTexture().draw(this->previewBounds[render::Layout::Front]);
					}
				}
			}

			if (!this->controlsVisible) return;

			this->guiSettings.mouseOverGui = this->canvas[render::Layout::Back]->isEditing() || this->canvas[render::Layout::Front]->isEditing();
			this->guiSettings.windowPos = ofVec2f(kGuiMargin, kGuiMargin);
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
				scene->getPostParameters(layout).screenRatio = this->screenBounds[layout].getAspectRatio();

				// Draw the base and world content.
				this->canvas[layout]->beginDraw();
				{
					this->playlist->drawSceneBase(layout);
					this->playlist->drawSceneWorld(layout);
				}
				this->canvas[layout]->endDraw();

				// Post-process the content, either directly in the Scene or in the Canvas.
				const auto postProcessing = this->playlist->postProcess(layout, this->canvas[layout]->getDrawTexture(), this->canvas[layout]->getPostFbo());
				if (!postProcessing)
				{
					this->canvas[layout]->postProcess(scene->getPostParameters(layout));
				}

				// Draw the overlay content.
				this->canvas[layout]->beginDraw();
				{
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
		void App_::drawGui(ofxPreset::Gui::Settings & settings)
		{
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), this->guiSettings))
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

				ofxPreset::Gui::AddParameter(this->parameters.background);

				if (ofxPreset::Gui::BeginTree(this->parameters.controlScreen, settings))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.controlScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.controlScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.controlScreen.screenHeight);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					if (ofxPreset::Gui::BeginTree(this->parameters.controlScreen.preview, settings))
					{
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.backEnabled);
						ImGui::SameLine();
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.frontEnabled);
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.scale);

						ofxPreset::Gui::EndTree(settings);
					}

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->parameters.backScreen, settings))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.backScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.screenHeight);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.numRows);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.numCols);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->parameters.frontScreen, settings))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.frontScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.screenHeight);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.numRows);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.numCols);
					if (ImGui::Button("Apply"))
					{
						this->applyConfiguration();
					}

					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(this->guiSettings);
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
				// Fit the Canvas previews for the Control screen.
				this->previewBounds[render::Layout::Back].height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				this->previewBounds[render::Layout::Back].width = this->canvas[render::Layout::Back]->getWidth() * this->previewBounds[render::Layout::Back].height / this->canvas[render::Layout::Back]->getHeight();

				this->previewBounds[render::Layout::Front].width = this->previewBounds[render::Layout::Back].width * this->canvas[render::Layout::Front]->getWidth() / this->canvas[render::Layout::Back]->getWidth();
				this->previewBounds[render::Layout::Front].height = this->canvas[render::Layout::Front]->getHeight() * this->previewBounds[render::Layout::Front].width / this->canvas[render::Layout::Front]->getWidth();

				this->previewBounds[render::Layout::Back].x = (this->boundsControl.getWidth() - this->previewBounds[render::Layout::Back].getWidth()) * 0.5f;
				this->previewBounds[render::Layout::Front].x = (this->boundsControl.getWidth() - this->previewBounds[render::Layout::Front].getWidth()) * 0.5f;

				this->previewBounds[render::Layout::Back].y = this->boundsControl.getMinY() + kGuiMargin;
				this->previewBounds[render::Layout::Front].y = this->previewBounds[render::Layout::Back].getMaxY() + kGuiMargin;

				// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
				this->playlist->setCameraControlArea(render::Layout::Back, this->previewBounds[render::Layout::Back]);
				this->playlist->setCameraControlArea(render::Layout::Front, this->previewBounds[render::Layout::Front]);
			}
			else
			{
				// Set the Scene cameras to use the Canvas bounds as mouse-enabled areas.
				this->playlist->setCameraControlArea(render::Layout::Back, this->screenBounds[render::Layout::Back]);
				this->playlist->setCameraControlArea(render::Layout::Front, this->screenBounds[render::Layout::Front]);
			}
		}
		
		//--------------------------------------------------------------
		void App_::onKeyPressed(ofKeyEventArgs & args)
		{
			if (args.keycode == GLFW_KEY_F && (ofGetKeyPressed(OF_KEY_CONTROL) || ofGetKeyPressed(OF_KEY_COMMAND)))
			{
				ofToggleFullscreen();
				return;
			}
			if (args.key == '`')
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
