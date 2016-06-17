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
			
			// Register events listeners.
			ofAddListener(ofEvents().update, this, &App_::onUpdate);
			ofAddListener(ofEvents().draw, this, &App_::onDraw);
			
			ofAddListener(ofEvents().keyPressed, this, &App_::onKeyPressed);
			ofAddListener(ofEvents().keyReleased, this, &App_::onKeyReleased);

			ofAddListener(ofEvents().mouseMoved, this, &App_::onMouseMoved);
			ofAddListener(ofEvents().mousePressed, this, &App_::onMousePressed);
			ofAddListener(ofEvents().mouseDragged, this, &App_::onMouseDragged);
			ofAddListener(ofEvents().mouseReleased, this, &App_::onMouseReleased);

			ofAddListener(this->canvas->resizeEvent, this, &App_::onCanvasResized);
			ofAddListener(ofEvents().windowResized, this, &App_::onWindowResized);
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

			ofRemoveListener(this->canvas->resizeEvent, this, &App_::onCanvasResized);
			ofRemoveListener(ofEvents().windowResized, this, &App_::onWindowResized);

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

			auto dt = ofGetLastFrameTime();
			this->sceneManager->update(dt);
		}
		
		//--------------------------------------------------------------
		void App_::onDraw(ofEventArgs & args)
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
				this->guiSettings.mouseOverGui = this->canvas->isEditing();
				this->guiSettings.windowPos = ofVec2f(kGuiMargin, kGuiMargin);
				this->guiSettings.windowSize = ofVec2f::zero();

				if (ofxPreset::Gui::BeginWindow("App", this->guiSettings))
				{
					ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
				}
				ofxPreset::Gui::EndWindow(this->guiSettings);

				this->canvas->drawGui(this->guiSettings);
				this->sceneManager->drawGui(this->guiSettings);
			}
			this->imGui.end();

			// Draw the non-gui overlay.
			this->sceneManager->drawOverlay(this->guiSettings);
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
			if (this->canvas->keyPressed(args))
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
			this->canvas->cursorMoved(args);
		}

		//--------------------------------------------------------------
		void App_::onMousePressed(ofMouseEventArgs & args)
		{
			this->canvas->cursorDown(args);
		}

		//--------------------------------------------------------------
		void App_::onMouseDragged(ofMouseEventArgs & args)
		{
			this->canvas->cursorDragged(args);
		}

		//--------------------------------------------------------------
		void App_::onMouseReleased(ofMouseEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void App_::onCanvasResized(ofResizeEventArgs & args)
		{
			this->sceneManager->canvasResized(args);
		}

		//--------------------------------------------------------------
		void App_::onWindowResized(ofResizeEventArgs & args)
		{

		}
	}
}