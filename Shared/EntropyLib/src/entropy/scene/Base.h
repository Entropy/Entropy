#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "Mapping.h"

#define ENTROPY_SCENE_SETUP_LISTENER \
	onSetupListeners.push_back(this->onSetup.newListener([this]() { \
		this->setup(); \
	}));
#define ENTROPY_SCENE_EXIT_LISTENER \
	onExitListeners.push_back(this->onExit.newListener([this]() { \
		this->exit(); \
	}));
#define ENTROPY_SCENE_RESIZE_LISTENER \
	onResizeListeners.push_back(this->onResize.newListener([this](ofResizeEventArgs & args) { \
		this->resize(args); \
	}));
#define ENTROPY_SCENE_UPDATE_LISTENER \
	onUpdateListeners.push_back(this->onUpdate.newListener([this](double & dt) { \
		this->update(dt); \
	}));
#define ENTROPY_SCENE_DRAW_BACK_LISTENER \
	onDrawBackListeners.push_back(this->onDrawBack.newListener([this]() { \
		this->drawBack(); \
	}));
#define ENTROPY_SCENE_DRAW_WORLD_LISTENER \
	onDrawWorldListeners.push_back(this->onDrawWorld.newListener([this]() { \
		this->drawWorld(); \
	}));
#define ENTROPY_SCENE_DRAW_FRONT_LISTENER \
	onDrawFrontListeners.push_back(this->onDrawFront.newListener([this]() { \
		this->drawFront(); \
	}));
#define ENTROPY_SCENE_GUI_LISTENER \
	onGuiListeners.push_back(this->onGui.newListener([this](ofxPreset::GuiSettings & settings) { \
		this->gui(settings); \
	}));
#define ENTROPY_SCENE_SERIALIZATION_LISTENERS \
	onSerializeListeners.push_back(this->onSerialize.newListener([this](nlohmann::json & json) { \
		this->serialize(json); \
	})); \
	onDeserializeListeners.push_back(this->onDeserialize.newListener([this](const nlohmann::json & json) { \
		this->deserialize(json); \
	}));

namespace entropy
{
	namespace scene
	{
		class Base
		{
		public:
			virtual string getName() const = 0;
			virtual ofCamera & getCamera();

			Base();
			virtual ~Base();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double dt);
			void draw();

			// Parameters
			void gui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			// Resources
            string getAssetsPath(const string & file = "");
			string getDataPath(const string & file = "");
			string getPresetPath(const string & preset = "");

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			// Timeline
			void drawTimeline(ofxPreset::GuiSettings & settings);

			void setCameraLocked(bool cameraLocked);
			void toggleCameraLocked();
			bool isCameraLocked() const;

			void addCameraKeyframe();

		protected:
			void drawBack();
			void drawWorld();
			void drawFront();

			// Events
			ofEvent<void> onSetup;
			ofEvent<void> onExit;
			ofEvent<ofResizeEventArgs> onResize;

			ofEvent<double> onUpdate;

			ofEvent<void> onDrawBack;
			ofEvent<void> onDrawWorld;
			ofEvent<void> onDrawFront;

			ofEvent<ofxPreset::GuiSettings> onGui;

			ofEvent<nlohmann::json> onSerialize;
			ofEvent<const nlohmann::json> onDeserialize;

			vector<ofEventListener> onSetupListeners;
			vector<ofEventListener> onExitListeners;
			vector<ofEventListener> onResizeListeners;

			vector<ofEventListener> onUpdateListeners;

			vector<ofEventListener> onDrawBackListeners;
			vector<ofEventListener> onDrawWorldListeners;
			vector<ofEventListener> onDrawFrontListeners;

			vector<ofEventListener> onGuiListeners;

			vector<ofEventListener> onSerializeListeners;
			vector<ofEventListener> onDeserializeListeners;

			// Resources
			void populatePresets();

            string assetsPath;
			string dataPath;
			string currPreset;
			vector<string> presets;

			// Parameters
			struct BaseParameters
				: ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<ofFloatColor> background{ "Background", ofFloatColor::black };

					PARAM_DECLARE("Base", background);
				} base;

				PARAM_DECLARE("Parameters", base);
			};

			virtual BaseParameters & getParameters() = 0;

			// Timeline
			ofxTimeline timeline;
			ofxTLCameraTrack * cameraTrack;
			map<string, shared_ptr<AbstractMapping>> mappings;

		private:
			// Timeline
			void populateMappings(const ofParameterGroup & group, string name = "");
			void refreshMappings();

			// Camera
			ofEasyCam camera;
		};
	}
}