#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxLiquidEvent.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "Mapping.h"

#define ENTROPY_SCENE_SETUP_LISTENER \
	this->onSetup += [this]() { \
		this->setup(); \
	}
#define ENTROPY_SCENE_EXIT_LISTENER \
	this->onExit += [this]() { \
		this->exit(); \
	}
#define ENTROPY_SCENE_UPDATE_LISTENER \
	this->onUpdate += [this]() { \
		this->update(); \
	}
#define ENTROPY_SCENE_DRAW_BACK_LISTENER \
	this->onDrawBack += [this]() { \
		this->drawBack(); \
	}
#define ENTROPY_SCENE_DRAW_WORLD_LISTENER \
	this->onDrawWorld += [this]() { \
		this->drawWorld(); \
	}
#define ENTROPY_SCENE_DRAW_FRONT_LISTENER \
	this->onDrawFront += [this]() { \
		this->drawFront(); \
	}
#define ENTROPY_SCENE_GUI_LISTENER \
	this->onGui += [this](ofxPreset::GuiSettings & settings) { \
		this->gui(settings); \
	}
#define ENTROPY_SCENE_SERIALIZATION_LISTENERS \
	this->onSerialize += [this](nlohmann::json & json) { \
		this->serialize(json); \
	}; \
	this->onDeserialize += [this](const nlohmann::json & json) { \
		this->deserialize(json); \
	}

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

			void update();
			void draw();

			// Parameters
			void gui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			// Resources
			string getDataPath(const string & file = "");
			string getPresetPath(const string & preset = "");

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			// Overlays
			void setOverlayVisible(bool overlayVisible);
			void toggleOverlayVisible();
			bool isOverlayVisible() const;

		protected:
			void drawBack();
			void drawWorld();
			void drawFront();

			// Events
			ofxLiquidEvent<void> onSetup;
			ofxLiquidEvent<void> onExit;

			ofxLiquidEvent<void> onUpdate;

			ofxLiquidEvent<void> onDrawBack;
			ofxLiquidEvent<void> onDrawWorld;
			ofxLiquidEvent<void> onDrawFront;

			ofxLiquidEvent<ofxPreset::GuiSettings> onGui;

			ofxLiquidEvent<nlohmann::json> onSerialize;
			ofxLiquidEvent<const nlohmann::json> onDeserialize;

			// Resources
			void populatePresets();

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

			// Overlays
			ofxPreset::GuiSettings guiSettings;

			ofxTimeline timeline;
			map<string, shared_ptr<AbstractMapping>> mappings;

		private:
			// Overlays
			void drawOverlay();

			void populateMappings(const ofParameterGroup & group, string name = "");
			void refreshMappings();

			static ofxImGui imgui;
			bool overlayVisible;

			// Camera
			ofEasyCam camera;
		};
	}
}