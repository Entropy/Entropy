#pragma once

#include "ofFileUtils.h"

#include "entropy/render/Layout.h"
#include "entropy/world/Camera.h"

#include "Base.h"

namespace entropy
{
	namespace scene
	{
		class Playlist
		{
		public:
			Playlist();
			~Playlist();

			bool addScene(shared_ptr<Base> scene);
			bool removeScene(shared_ptr<Base> scene);
			bool removeScene(const string & name);

			void previewScene();

			shared_ptr<Base> getScene(const string & name);
			template<typename SceneType>
			shared_ptr<SceneType> getScene(const string & name);

			shared_ptr<Base> getCurrentScene() const;
			template<typename SceneType>
			shared_ptr<SceneType> getCurrentScene();

			const string & getCurrentPresetName() const;

			void addTrack(const string & sceneName, const string & presetName);
			void removeTrack();

			bool playTrack(size_t index);
			bool stopTrack();
			size_t getCurrentTrack() const;

			bool update(double dt);

			bool drawScene(render::Layout layout);

			bool drawGui(ofxImGui::Settings & settings);
			bool drawTimeline(ofxImGui::Settings & settings);

			bool keyPressed(ofKeyEventArgs & args);

			void canvasResized(render::Layout layout, ofResizeEventArgs & args);

			const std::filesystem::path & getDataPath();
			const std::filesystem::path & getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			bool setCurrentScene(const string & name);
			bool setCurrentPreset(const string & name, bool showtime);
			void unsetCurrent();

			map<string, shared_ptr<Base>> scenes;
			shared_ptr<Base> currentScene;

			map<string, string> shortNames;
			vector<pair<string, string>> tracks;
			size_t currentTrack;

			ofEventListener presetCuedListener;
			ofEventListener presetLoadedListener;
			ofEventListener presetSavedListener;
			string nextPreset;
		};
	}
}
