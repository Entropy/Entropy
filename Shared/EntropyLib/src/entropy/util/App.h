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
			void update(ofEventArgs & args);
			void draw(ofEventArgs & args);
			void keyPressed(ofKeyEventArgs & args);

		protected:
			shared_ptr<entropy::render::Canvas> canvas;
			shared_ptr<entropy::scene::Manager> sceneManager;

			ofxImGui imGui;
			ofxPreset::GuiSettings guiSettings;
			bool overlayVisible;
		};

		typedef entropy::util::Singleton<App_> App;
	}
}