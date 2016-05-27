#pragma once

#include "ofMain.h"

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

		protected:
			// OF
			void update(ofEventArgs & args);
			void draw(ofEventArgs & args);
			void keyPressed(ofKeyEventArgs & args);

			// GUI
			void gui();

			ofxImGui imgui;
			bool guiVisible;
			ofxPreset::GuiSettings guiSettings;

			// Scenes
			map<string, shared_ptr<Base>> scenes;
			shared_ptr<Base> currentScene;
		};
	}
}