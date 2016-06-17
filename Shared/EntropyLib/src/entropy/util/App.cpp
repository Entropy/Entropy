#include "App.h"

namespace entropy
{
	namespace util
	{
		//--------------------------------------------------------------
		App_::App_()
		{
			this->canvas = make_shared<entropy::render::Canvas>();
			this->sceneManager = make_shared<entropy::scene::Manager>();

			this->imGui.setup();
			this->overlayVisible = true;
			
			// Register OF events.
			ofAddListener(ofEvents().update, this, &App_::update);
			ofAddListener(ofEvents().draw, this, &App_::draw);
			ofAddListener(ofEvents().keyPressed, this, &App_::keyPressed);

			// Register custom events.
			ofAddListener(this->canvas->resizeEvent, this, &App_::canvasResized);
		}

		//--------------------------------------------------------------
		App_::~App_()
		{
			// Unregister custom events.
			ofRemoveListener(this->canvas->resizeEvent, this, &App_::canvasResized);
			
			// Unregister OF events.
			ofRemoveListener(ofEvents().update, this, &App_::update);
			ofRemoveListener(ofEvents().draw, this, &App_::draw);
			ofRemoveListener(ofEvents().keyPressed, this, &App_::keyPressed);

			this->imGui.close();

			// Reset pointers.
			this->canvas.reset();
			this->sceneManager.reset();
		}

		//--------------------------------------------------------------
		shared_ptr<entropy::render::Canvas> App_::getCanvas()
		{
			return this->canvas;
		}

		//--------------------------------------------------------------
		shared_ptr<entropy::scene::Manager> App_::getSceneManager()
		{
			return this->sceneManager;
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
		void App_::update(ofEventArgs & args)
		{
			if (this->overlayVisible || ofGetWindowMode() == OF_WINDOW)
			{
				ofShowCursor();
			}
			else
			{
				ofHideCursor();
			}

			auto dt = ofGetLastFrameTime();
			this->sceneManager->update(dt);
		}
		
		//--------------------------------------------------------------
		void App_::draw(ofEventArgs & args)
		{
			// Draw the content.
			this->canvas->begin();
			{
				this->sceneManager->drawScene();
			}
			this->canvas->end();

			this->canvas->draw();

			if (!this->overlayVisible) return;

			// Draw the gui.
			this->imGui.begin();
			{
				this->guiSettings.mouseOverGui = false;
				this->guiSettings.windowPos = ofVec2f(kGuiMargin, kGuiMargin);
				this->guiSettings.windowSize = ofVec2f::zero();

				if (ofxPreset::Gui::BeginWindow("App", this->guiSettings))
				{
					ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
				}
				ofxPreset::Gui::EndWindow(this->guiSettings);

				this->sceneManager->drawGui(this->guiSettings);
			}
			this->imGui.end();

			// Draw the non-gui overlay.
			this->sceneManager->drawOverlay(this->guiSettings);
		}
		
		//--------------------------------------------------------------
		void App_::keyPressed(ofKeyEventArgs & args)
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
			if (this->sceneManager->keyPressed(args))
			{
				return;
			}
		}

		//--------------------------------------------------------------
		void App_::canvasResized(ofResizeEventArgs & args)
		{
			this->sceneManager->canvasResized(args);
		}
	}
}