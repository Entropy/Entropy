#pragma once

#include "ofMain.h"

#include "entropy/render/Canvas.h"
#include "entropy/scene/Manager.h"
#include "entropy/util/Singleton.h"

namespace entropy
{
	namespace util
	{
		class App_
		{
		public:
			App_();
			~App_();

			shared_ptr<entropy::render::Canvas> getCanvas();
			shared_ptr<entropy::scene::Manager> getSceneManager();

			bool isMouseOverGui() const;
			bool isOverlayVisible() const;

		protected:
			void onUpdate(ofEventArgs & args);
			void onDraw(ofEventArgs & args);

			void onMouseMoved(ofMouseEventArgs & args);
			void onMousePressed(ofMouseEventArgs & args);
			void onMouseDragged(ofMouseEventArgs & args);
			void onMouseReleased(ofMouseEventArgs & args);

			void onKeyPressed(ofKeyEventArgs & args);
			void onKeyReleased(ofKeyEventArgs & args);

			void onCanvasResized(ofResizeEventArgs & args);
			void onWindowResized(ofResizeEventArgs & args);

		protected:
			shared_ptr<entropy::render::Canvas> canvas;
			shared_ptr<entropy::scene::Manager> sceneManager;

			ofxImGui imGui;
			ofxPreset::Gui::Settings guiSettings;
			bool overlayVisible;
		};

		typedef entropy::util::Singleton<App_> App;
	}
}