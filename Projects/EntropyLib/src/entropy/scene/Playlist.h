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

			bool isActive() const;

			shared_ptr<Base> getCurrentScene() const;
			template<typename SceneType>
			shared_ptr<SceneType> getCurrentScene();

			const string & getCurrentPresetName() const;

			bool addTrack(const string & sceneName, const string & presetName);
			bool removeTrack();

			void preloadTracks();

			bool playTrack(size_t index);
			bool stopTrack();
			size_t getCurrentTrack() const;

			bool update(double dt);

			bool drawSceneBase(render::Layout layout);
			bool drawSceneWorld(render::Layout layout);
			bool drawSceneOverlay(render::Layout layout);

			bool drawGui(ofxImGui::Settings & settings);
			bool drawTimeline(ofxImGui::Settings & settings);

			bool keyPressed(ofKeyEventArgs & args);

			void setCameraControlArea(render::Layout layout, const ofRectangle & controlArea);

			const world::Camera::Settings & getCameraSettings(render::Layout layout);

			void canvasResized(render::Layout layout, ofResizeEventArgs & args);

			const std::filesystem::path & getDataPath();
			const std::filesystem::path & getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			std::vector<std::pair<std::shared_ptr<Base>, std::string>> tracks;
			size_t currentTrack;
			std::shared_ptr<Base> currentScene;

			ofEventListener presetCuedListener;
			ofEventListener presetLoadedListener;
			ofEventListener presetSavedListener;
			string nextPreset;

			map<render::Layout, ofRectangle> cameraControlAreas;
			map<render::Layout, world::Camera::Settings> cameraSettings;
		};
	}
}
