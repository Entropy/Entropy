#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "entropy/popup/Image.h"
#include "entropy/popup/Video.h"
#include "entropy/util/Mapping.h"

#define ENTROPY_SCENE_SETUP_LISTENER \
	this->onSetupListeners.push_back(this->onSetup.newListener([this]() { \
		this->setup(); \
	}));
#define ENTROPY_SCENE_EXIT_LISTENER \
	this->onExitListeners.push_back(this->onExit.newListener([this]() { \
		this->exit(); \
	}));
#define ENTROPY_SCENE_RESIZE_LISTENER \
	this->onResizeListeners.push_back(this->onResize.newListener([this](ofResizeEventArgs & args) { \
		this->resize(args); \
	}));
#define ENTROPY_SCENE_UPDATE_LISTENER \
	this->onUpdateListeners.push_back(this->onUpdate.newListener([this](double & dt) { \
		this->update(dt); \
	}));
#define ENTROPY_SCENE_DRAW_BACK_LISTENER \
	this->onDrawBackListeners.push_back(this->onDrawBack.newListener([this]() { \
		this->drawBack(); \
	}));
#define ENTROPY_SCENE_DRAW_WORLD_LISTENER \
	this->onDrawWorldListeners.push_back(this->onDrawWorld.newListener([this]() { \
		this->drawWorld(); \
	}));
#define ENTROPY_SCENE_DRAW_FRONT_LISTENER \
	this->onDrawFrontListeners.push_back(this->onDrawFront.newListener([this]() { \
		this->drawFront(); \
	}));
#define ENTROPY_SCENE_GUI_LISTENER \
	this->onGuiListeners.push_back(this->onGui.newListener([this](ofxPreset::Gui::Settings & settings) { \
		this->gui(settings); \
	}));
#define ENTROPY_SCENE_SERIALIZATION_LISTENERS \
	this->onSerializeListeners.push_back(this->onSerialize.newListener([this](nlohmann::json & json) { \
		this->serialize(json); \
	})); \
	this->onDeserializeListeners.push_back(this->onDeserialize.newListener([this](const nlohmann::json & json) { \
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

			Base();
			virtual ~Base();

			// Base methods
			void setup_();
			void exit_();
			void resize_(ofResizeEventArgs & args);

			void update_(double dt);

			void drawBackBase_();
			void drawBackWorld_();
			void drawBackOverlay_();

			void drawFrontBase_();
			void drawFrontWorld_();
			void drawFrontOverlay_();

			void gui_(ofxPreset::Gui::Settings & settings);

			void serialize_(nlohmann::json & json);
			void deserialize_(const nlohmann::json & json);

			// Override methods
			virtual void setup() {}
			virtual void exit() {}
			virtual void resize(ofResizeEventArgs & args) {}

			virtual void update(double dt) {}

			virtual void drawBackBase() {}
			virtual void drawBackWorld() {}
			virtual void drawBackOverlay() {}

			virtual void drawFrontBase() {}
			virtual void drawFrontWorld() {}
			virtual void drawFrontOverlay() {}

			virtual void gui(ofxPreset::Gui::Settings & settings) {}

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

			// Resources
			string getAssetsPath(const string & file = "");
			string getDataPath(const string & file = "");
			string getPresetPath(const string & preset = "");
			string getCurrentPresetPath(const string & file = "");

			const string & getCurrentPresetName() const;

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			// Timeline
			void drawTimeline(ofxPreset::Gui::Settings & settings);
			int getCurrentTimelineFrame();

			// Camera
			void setCameraLocked(bool cameraLocked);
			void toggleCameraLocked();
			bool isCameraLocked() const;

			void addCameraKeyframe();
			 
			// Post-Processing
			virtual bool postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo);

			// Export
			void beginExport();
			void endExport();

		protected:
			// Camera
			virtual void resetCamera();

			ofEasyCam cameraBack;
			ofEasyCam cameraFront;

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
					ofParameter<ofFloatColor> background{ "Background", ofFloatColor::black };
					struct : ofParameterGroup
					{
						ofParameter<bool> relativeYAxis{ "Relative Y Axis", true };
						ofParameter<bool> attachFrontToBack{ "Attach Front to Back", true };

						PARAM_DECLARE("Camera", relativeYAxis, attachFrontToBack);
					} camera;

					PARAM_DECLARE("Base", background, camera);
				} base;

				PARAM_DECLARE("Parameters", base);
			};

			virtual BaseParameters & getParameters() = 0;

			// Timeline
			ofxTimeline timeline;
			ofxTLCameraTrack * cameraTrack;
			map<string, shared_ptr<util::AbstractMapping>> mappings;

			// Popups
			shared_ptr<popup::Base> addPopUp(popup::Type type);
			void removePopUp();

			vector<shared_ptr<popup::Base>> popUps;

		private:
			// Timeline
			void populateMappings(const ofParameterGroup & group, string name = "");
			void refreshMappings();
		};
	}
}