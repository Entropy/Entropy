#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxLiquidEvent.h"
#include "ofxPreset.h"

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
#define ENTROPY_SCENE_DRAW_LISTENER \
	this->onDraw += [this]() { \
		this->draw(); \
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

			Base();
			virtual ~Base();

			void setup();
			void exit();

			void update();
			void draw();

			void gui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			// Events
			ofxLiquidEvent<void> onSetup;
			ofxLiquidEvent<void> onExit;

			ofxLiquidEvent<void> onUpdate;
			ofxLiquidEvent<void> onDraw;

			ofxLiquidEvent<ofxPreset::GuiSettings> onGui;

			ofxLiquidEvent<nlohmann::json> onSerialize;
			ofxLiquidEvent<const nlohmann::json> onDeserialize;

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
		};
	}
}