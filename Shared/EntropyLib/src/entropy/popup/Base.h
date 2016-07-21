#pragma once

#include "glm/glm.hpp"

#include "ofParameter.h"
#include "ofxPreset.h"

#define ENTROPY_POPUP_SETUP_LISTENER \
	this->onSetupListeners.push_back(this->onSetup.newListener([this]() { \
		this->setup(); \
	}));
#define ENTROPY_POPUP_EXIT_LISTENER \
	this->onExitListeners.push_back(this->onExit.newListener([this]() { \
		this->exit(); \
	}));
#define ENTROPY_POPUP_RESIZE_LISTENER \
	this->onResizeListeners.push_back(this->onResize.newListener([this](ofResizeEventArgs & args) { \
		this->resize(args); \
	}));
#define ENTROPY_POPUP_UPDATE_LISTENER \
	this->onUpdateListeners.push_back(this->onUpdate.newListener([this](double & dt) { \
		this->update(dt); \
	}));
#define ENTROPY_POPUP_DRAW_LISTENER \
	this->onDrawListeners.push_back(this->onDrawWorld.newListener([this]() { \
		this->drawWorld(); \
	}));
#define ENTROPY_POPUP_GUI_LISTENER \
	this->onGuiListeners.push_back(this->onGui.newListener([this](ofxPreset::Gui::Settings & settings) { \
		this->gui(settings); \
	}));
#define ENTROPY_POPUP_SERIALIZATION_LISTENERS \
	this->onSerializeListeners.push_back(this->onSerialize.newListener([this](nlohmann::json & json) { \
		this->serialize(json); \
	})); \
	this->onDeserializeListeners.push_back(this->onDeserialize.newListener([this](const nlohmann::json & json) { \
		this->deserialize(json); \
	}));

namespace entropy
{
	namespace popup
	{
		class Base
		{
		public:
			typedef enum
			{
				TYPE_UNKNOWN,
				TYPE_IMAGE
			} Type;

			Base(Type type = TYPE_UNKNOWN);
			virtual ~Base();

			Type getType() const;

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double dt);
			void draw();

			// Parameters
			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			bool editing;

		protected:
			virtual ofTexture & getTexture() = 0;

			void updateBounds();
			ofRectangle viewport;
			ofRectangle roi;
			bool boundsDirty;

			Type type;

		protected:
			// Events
			ofEvent<void> onSetup;
			ofEvent<void> onExit;
			ofEvent<ofResizeEventArgs> onResize;

			ofEvent<double> onUpdate;
			ofEvent<void> onDraw;

			ofEvent<ofxPreset::Gui::Settings> onGui;

			ofEvent<nlohmann::json> onSerialize;
			ofEvent<const nlohmann::json> onDeserialize;

			vector<ofEventListener> onSetupListeners;
			vector<ofEventListener> onExitListeners;
			vector<ofEventListener> onResizeListeners;

			vector<ofEventListener> onUpdateListeners;
			vector<ofEventListener> onDrawListeners;

			vector<ofEventListener> onGuiListeners;

			vector<ofEventListener> onSerializeListeners;
			vector<ofEventListener> onDeserializeListeners;

			// Parameters
			struct BaseParameters
				: ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofxPreset::Parameter<ofFloatColor> background{ "Background", ofFloatColor::black };
					ofxPreset::Parameter<glm::vec2> size{ "Size", glm::vec2(0.1f), glm::vec2(0.0f), glm::vec2(1.0f) };
					ofxPreset::Parameter<glm::vec2> center{ "Center", glm::vec2(0.5f), glm::vec2(0.0f), glm::vec2(1.0f) };

					PARAM_DECLARE("Base", background, size, center);
				} base;

				PARAM_DECLARE("Parameters", base);
			};

			virtual BaseParameters & getParameters() = 0;
		};
	}
}