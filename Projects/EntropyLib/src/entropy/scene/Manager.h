#pragma once

#include "ofMain.h"

#include "entropy/render/Layout.h"

#include "Base.h"

namespace entropy
{
	namespace scene
	{
		class Manager
		{
		public:
			Manager();
			~Manager();

			bool addScene(shared_ptr<Base> scene);
			bool removeScene(shared_ptr<Base> scene);
			bool removeScene(const string & name);

			shared_ptr<Base> getScene(const string & name);
			template<typename SceneType>
			shared_ptr<SceneType> getScene(const string & name);

			shared_ptr<Base> getCurrentScene();
			template<typename SceneType>
			shared_ptr<SceneType> getCurrentScene();

			bool setCurrentScene(const string & name);

			void update(double dt);

			void drawScene(render::Layout layout);
			void drawGui(ofxPreset::Gui::Settings & settings);
			void drawOverlay(ofxPreset::Gui::Settings & settings);

			bool postProcess(render::Layout layout, const ofTexture & srcTexture, const ofFbo & dstFbo) const;

			bool keyPressed(ofKeyEventArgs & args);

			void canvasResized(render::Layout layout, ofResizeEventArgs & args);

		protected:
			map<string, shared_ptr<Base>> scenes;
			shared_ptr<Base> currentScene;
		};
	}
}
