#pragma once

#include "ofMain.h"

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

			bool drawSceneBase(render::Layout layout);
			bool drawSceneWorld(render::Layout layout);
			bool drawSceneOverlay(render::Layout layout);

			bool drawGui(ofxPreset::Gui::Settings & settings);
			bool drawTimeline(ofxPreset::Gui::Settings & settings);

			bool postProcess(render::Layout layout, const ofTexture & srcTexture, const ofFbo & dstFbo) const;

			bool keyPressed(ofKeyEventArgs & args);

			void setCameraControlArea(render::Layout layout, const ofRectangle & controlArea);

			const world::Camera::Settings & getCameraSettings(render::Layout layout);

			void canvasResized(render::Layout layout, ofResizeEventArgs & args);

			const string & getDataPath();
			const string & getSettingsFilePath();

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

			map<render::Layout, ofRectangle> cameraControlAreas;
			map<render::Layout, world::Camera::Settings> cameraSettings;
		};
	}
}
