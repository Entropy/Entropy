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
			this->canvasBack = make_shared<render::Canvas>(render::Layout::Back);
			this->canvasFront = make_shared<render::Canvas>(render::Layout::Front);
			this->sceneManager = make_shared<scene::Manager>();

			// Setup gui.
			this->imGui.setup();
			this->overlayVisible = true;
			
			// Register events listeners.
			ofAddListener(ofEvents().update, this, &App_::onUpdate);
			ofAddListener(ofEvents().draw, this, &App_::onDraw);
			
			ofAddListener(ofEvents().keyPressed, this, &App_::onKeyPressed);
			ofAddListener(ofEvents().keyReleased, this, &App_::onKeyReleased);

			ofAddListener(ofEvents().mouseMoved, this, &App_::onMouseMoved);
			ofAddListener(ofEvents().mousePressed, this, &App_::onMousePressed);
			ofAddListener(ofEvents().mouseDragged, this, &App_::onMouseDragged);
			ofAddListener(ofEvents().mouseReleased, this, &App_::onMouseReleased);

			ofAddListener(this->canvasBack->resizeEvent, this, &App_::onCanvasBackResized);
			ofAddListener(this->canvasFront->resizeEvent, this, &App_::onCanvasFrontResized);
			ofAddListener(ofEvents().windowResized, this, &App_::onWindowResized);

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

			ofRemoveListener(this->canvasBack->resizeEvent, this, &App_::onCanvasBackResized);
			ofRemoveListener(this->canvasFront->resizeEvent, this, &App_::onCanvasFrontResized);
			ofRemoveListener(ofEvents().windowResized, this, &App_::onWindowResized);

			this->imGui.close();

			// Reset pointers.
			this->canvasBack.reset();
			this->canvasFront.reset();
			this->sceneManager.reset();
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
		shared_ptr<entropy::render::Canvas> App_::getCanvasBack() const
		{
			return this->canvasBack;
		}

		//--------------------------------------------------------------
		shared_ptr<entropy::render::Canvas> App_::getCanvasFront() const
		{
			return this->canvasFront;
		}

		//--------------------------------------------------------------
		shared_ptr<entropy::scene::Manager> App_::getSceneManager() const
		{
			return this->sceneManager;
		}

		//--------------------------------------------------------------
		const ofRectangle & App_::getBoundsControl() const
		{
			return this->boundsControl;
		}

		//--------------------------------------------------------------
		const ofRectangle & App_::getBoundsBack() const
		{
			return this->boundsBack;
		}

		//--------------------------------------------------------------
		const ofRectangle & App_::getBoundsFront() const
		{
			return this->boundsFront;
		}

		//--------------------------------------------------------------
		bool App_::isMouseOverGui() const
		{
			return this->guiSettings.mouseOverGui;
		}
		
		//--------------------------------------------------------------
		bool App_::isOverlayVisible() const
		{
			return this->overlayVisible;
		}

		//--------------------------------------------------------------
		void App_::onUpdate(ofEventArgs & args)
		{
			if (this->overlayVisible || ofGetWindowMode() == OF_WINDOW)
			{
				ofShowCursor();
			}
			else
			{
				ofHideCursor();
			}

			this->canvasBack->update();
			this->canvasFront->update();

			auto dt = ofGetLastFrameTime();
			this->sceneManager->update(dt);
		}
		
		//--------------------------------------------------------------
		void App_::onDraw(ofEventArgs & args)
		{
			ofBackground(this->parameters.background.get());

			// Back screen.
			if (this->parameters.backScreen.enabled || (this->parameters.controlScreen.enabled && this->parameters.controlScreen.preview.backEnabled))
			{
				// Draw the content.
				this->canvasBack->beginDraw();
				{
					this->sceneManager->drawScene(render::Layout::Back);
				}
				this->canvasBack->endDraw();

				// Post-process the content, either directly in the Scene or in the Canvas.
				const auto postProcessing = this->sceneManager->postProcess(render::Layout::Back, this->canvasBack->getDrawTexture(), this->canvasBack->getPostFbo());
				if (!postProcessing)
				{
					this->canvasBack->postProcess();
				}

				if (this->parameters.backScreen.enabled)
				{
					// Render the scene.
					this->canvasBack->render(this->boundsBack);
				}
			}

			// Front screen.
			if (this->parameters.frontScreen.enabled || (this->parameters.controlScreen.enabled && this->parameters.controlScreen.preview.frontEnabled))
			{
				// Draw the content.
				this->canvasFront->beginDraw();
				{
					this->sceneManager->drawScene(render::Layout::Front);
				}
				this->canvasFront->endDraw();

				// Post-process the content, either directly in the Scene or in the Canvas.
				const auto postProcessing = this->sceneManager->postProcess(render::Layout::Front, this->canvasFront->getDrawTexture(), this->canvasFront->getPostFbo());
				if (!postProcessing)
				{
					this->canvasFront->postProcess();
				}

				if (this->parameters.frontScreen.enabled)
				{
					// Render the scene.
					this->canvasFront->render(this->boundsFront);
				}
			}
			
			// Control screen.
			if (this->parameters.controlScreen.enabled)
			{
				if (this->parameters.controlScreen.preview.backEnabled)
				{
					this->canvasBack->getRenderTexture().draw(this->previewBoundsBack);
				}
				if (this->parameters.controlScreen.preview.frontEnabled)
				{
					this->canvasFront->getRenderTexture().draw(this->previewBoundsFront);
				}
			}

			if (!this->overlayVisible) return;

			this->guiSettings.mouseOverGui = this->canvasBack->isEditing() || this->canvasFront->isEditing();
			this->guiSettings.windowPos = ofVec2f(kGuiMargin, kGuiMargin);
			this->guiSettings.windowSize = ofVec2f::zero();
			if (this->parameters.controlScreen.enabled)
			{
				this->guiSettings.screenBounds = this->boundsControl;
			}
			else if (this->parameters.backScreen.enabled)
			{
				this->guiSettings.screenBounds = this->boundsBack;
			}
			else if (this->parameters.frontScreen.enabled)
			{
				this->guiSettings.screenBounds = this->boundsFront;
			}
			else
			{
				this->guiSettings.screenBounds = ofRectangle(0.0f, 0.0f, ofGetWidth(), ofGetHeight());
			}

			// Draw the gui overlay.
			this->imGui.begin();
			{
				this->drawGui(this->guiSettings);

				this->canvasBack->drawGui(this->guiSettings);
				this->canvasFront->drawGui(this->guiSettings);
				this->sceneManager->drawGui(this->guiSettings);
			}
			this->imGui.end();

			// Draw the non-gui overlay.
			this->sceneManager->drawOverlay(this->guiSettings);
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

				if (ImGui::CollapsingHeader(this->parameters.controlScreen.getName().c_str(), nullptr, true, true))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.controlScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.controlScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.controlScreen.screenHeight);
					if (ImGui::Button(ofxPreset::Gui::GetUniqueName("Apply")))
					{
						this->applyConfiguration();
					}

					ImGui::SetNextTreeNodeOpen(true);
					if (ImGui::TreeNode(this->parameters.controlScreen.preview.getName().c_str()))
					{
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.backEnabled);
						ImGui::SameLine();
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.frontEnabled);
						ofxPreset::Gui::AddParameter(this->parameters.controlScreen.preview.scale);

						ImGui::TreePop();
					}
				}

				if (ImGui::CollapsingHeader(this->parameters.backScreen.getName().c_str(), nullptr, true, true))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.backScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.screenHeight);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.numRows);
					ofxPreset::Gui::AddStepper(this->parameters.backScreen.numCols);
					if (ImGui::Button(ofxPreset::Gui::GetUniqueName("Apply")))
					{
						this->applyConfiguration();
					}
				}

				if (ImGui::CollapsingHeader(this->parameters.frontScreen.getName().c_str(), nullptr, true, true))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.frontScreen.enabled))
					{
						this->applyConfiguration();
					}
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.screenWidth);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.screenHeight);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.numRows);
					ofxPreset::Gui::AddStepper(this->parameters.frontScreen.numCols);
					if (ImGui::Button(ofxPreset::Gui::GetUniqueName("Apply")))
					{
						this->applyConfiguration();
					}
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

			this->boundsBack = ofRectangle(totalBounds.getMaxX(), totalBounds.getMinY(), this->parameters.backScreen.screenWidth * this->parameters.backScreen.numCols, this->parameters.backScreen.screenHeight * this->parameters.backScreen.numRows);
			if (this->parameters.backScreen.enabled)
			{
				totalBounds.growToInclude(this->boundsBack);
			}

			this->boundsFront = ofRectangle(totalBounds.getMaxX(), totalBounds.getMinY(), this->parameters.frontScreen.screenWidth * this->parameters.frontScreen.numCols, this->parameters.frontScreen.screenHeight * this->parameters.frontScreen.numRows);
			if (this->parameters.frontScreen.enabled)
			{
				totalBounds.growToInclude(this->boundsFront);
			}

			// Resize the window.
			ofSetWindowShape(totalBounds.width, totalBounds.height);

			// Force call the callback, in case the actual application window doesn't resize.
			// This happens if there aren't enough screens connected to span the whole thing.
			auto dummyArgs = ofResizeEventArgs();
			this->onWindowResized(dummyArgs);
		}

		//--------------------------------------------------------------
		void App_::updatePreviews()
		{
			if (this->parameters.controlScreen.enabled)
			{
				// Fit the Canvas previews for the Control screen.
				this->previewBoundsBack.height = this->boundsControl.getHeight() * this->parameters.controlScreen.preview.scale;
				this->previewBoundsBack.width = this->canvasBack->getWidth() * this->previewBoundsBack.height / this->canvasBack->getHeight();

				this->previewBoundsFront.width = this->previewBoundsBack.width * this->canvasFront->getWidth() / this->canvasBack->getWidth();
				this->previewBoundsFront.height = (this->canvasFront->getHeight() * this->previewBoundsFront.width) / this->canvasFront->getWidth();

				this->previewBoundsBack.x = (this->boundsControl.getWidth() - this->previewBoundsBack.getWidth()) * 0.5f;
				this->previewBoundsFront.x = (this->boundsControl.getWidth() - this->previewBoundsFront.getWidth()) * 0.5f;

				this->previewBoundsBack.y = this->boundsControl.getMinY() + kGuiMargin;
				this->previewBoundsFront.y = this->previewBoundsBack.getMaxY() + kGuiMargin;
			
				// Set the Scene cameras to use the Control screen previews as mouse-enabled areas.
				this->sceneManager->setCameraControlArea(render::Layout::Back, this->previewBoundsBack);
				this->sceneManager->setCameraControlArea(render::Layout::Front, this->previewBoundsFront);
			}
			else
			{
				// Set the Scene cameras to use the Canvas bounds as mouse-enabled areas.
				this->sceneManager->setCameraControlArea(render::Layout::Back, this->boundsBack);
				this->sceneManager->setCameraControlArea(render::Layout::Front, this->boundsFront);

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
				this->overlayVisible ^= 1;
			}
			if (this->canvasBack->keyPressed(args))
			{
				return;
			}
			if (this->canvasFront->keyPressed(args))
			{
				return;
			}
			if (this->sceneManager->keyPressed(args))
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
			if (this->boundsBack.inside(args))
			{
				args.x -= this->boundsBack.getMinX();
				args.y -= this->boundsBack.getMinY();
				this->canvasBack->cursorMoved(args);
			}
			else if (this->boundsFront.inside(args))
			{
				args.x -= this->boundsFront.getMinX();
				args.y -= this->boundsFront.getMinY();
				this->canvasFront->cursorMoved(args);
			}
		}

		//--------------------------------------------------------------
		void App_::onMousePressed(ofMouseEventArgs & args)
		{
			if (this->boundsBack.inside(args))
			{
				args.x -= this->boundsBack.getMinX();
				args.y -= this->boundsBack.getMinY();
				this->canvasBack->cursorDown(args);
			}
			else if (this->boundsFront.inside(args))
			{
				args.x -= this->boundsFront.getMinX();
				args.y -= this->boundsFront.getMinY();
				this->canvasFront->cursorDown(args);
			}
		}

		//--------------------------------------------------------------
		void App_::onMouseDragged(ofMouseEventArgs & args)
		{
			if (this->boundsBack.inside(args))
			{
				args.x -= this->boundsBack.getMinX();
				args.y -= this->boundsBack.getMinY();
				this->canvasBack->cursorDragged(args);
			}
			else if (this->boundsFront.inside(args))
			{
				args.x -= this->boundsFront.getMinX();
				args.y -= this->boundsFront.getMinY();
				this->canvasFront->cursorDragged(args);
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

			this->sceneManager->canvasResized(render::Layout::Back, args);
		}

		//--------------------------------------------------------------
		void App_::onCanvasFrontResized(ofResizeEventArgs & args)
		{
			this->updatePreviews();
			
			this->sceneManager->canvasResized(render::Layout::Front, args);
		}

		//--------------------------------------------------------------
		void App_::onWindowResized(ofResizeEventArgs & args)
		{
			this->updatePreviews();

			// Notify listeners.
			auto resizeBackArgs = ofResizeEventArgs(this->boundsBack.width, this->boundsBack.height);
			this->canvasBack->screenResized(resizeBackArgs);

			auto resizeFrontArgs = ofResizeEventArgs(this->boundsFront.width, this->boundsFront.height);
			this->canvasFront->screenResized(resizeFrontArgs);
		}
	}
}