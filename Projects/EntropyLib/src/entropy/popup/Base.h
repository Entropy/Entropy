#pragma once

#include "ofParameter.h"
#include "ofxPreset.h"
#include "ofxTLSwitches.h"

#include "entropy/render/Layout.h"

namespace entropy
{
	namespace popup
	{
		static const string PopUpsTimelinePageName = "Pop-ups";
		
		enum class Type
		{
			Unknown,
			Image,
			Video,
			Sound
		};

		enum class Surface
		{
			Base,
			Overlay
		};

		enum class Transition
		{
			Cut,
			Mix,
			Wipe,
			Strobe
		};
		
		class Base
		{
		public:
			Base(Type type = Type::Unknown);
			virtual ~Base();

			Type getType() const;
			render::Layout getLayout();
			Surface getSurface();

			bool editing;

			// Base methods
			void init_(int index, std::shared_ptr<ofxTimeline> timeline);
			void clear_();

			void setup_();
			void exit_();
			void resize_(ofResizeEventArgs & args);

			void update_(double dt);
			void draw_();

			void gui_(ofxPreset::Gui::Settings & settings);
			
			void serialize_(nlohmann::json & json);
			void deserialize_(const nlohmann::json & json);

			// Override methods
			virtual void init() {}
			virtual void clear() {}

			virtual void setup() {}
			virtual void exit() {}

			virtual void resize(ofResizeEventArgs & args) {}

			virtual void update(double dt) {}

			virtual void draw() {}

			virtual void gui(ofxPreset::Gui::Settings & settings) {}

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

		protected:
			virtual bool isLoaded() const = 0;

			virtual float getContentWidth() const = 0;
			virtual float getContentHeight() const = 0;
			virtual void renderContent() = 0;

			Type type;
			int index;

			void updateBounds();
			ofRectangle viewport;
			ofRectangle roi;
			bool boundsDirty;

			void updateBorder();
			bool borderDirty;

			float transitionPct;
			float switchMillis;

			// Timeline
			void addTimelineTrack();
			void removeTimelineTrack();

			// Per-frame attributes.
			float frontAlpha;
			ofRectangle srcBounds;
			ofRectangle dstBounds;
			ofVboMesh borderMesh;
	
		protected:
			// Timeline
			std::shared_ptr<ofxTimeline> timeline;
			ofxTLSwitches * switchesTrack;
			bool enabled;

			// Parameters
			struct BaseParameters
				: ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<int> layout{ "Layout", static_cast<int>(render::Layout::Front), static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Front) };
					ofParameter<int> surface{ "Surface", static_cast<int>(Surface::Overlay), static_cast<int>(Surface::Base), static_cast<int>(Surface::Overlay) };
					ofParameter<ofFloatColor> background{ "Background", ofFloatColor::black };
					ofParameter<float> size{ "Size", 0.1f, 0.0f, 1.0f };
					ofParameter<glm::vec2> center{ "Center", glm::vec2(0.5f), glm::vec2(0.0f), glm::vec2(1.0f) };

					PARAM_DECLARE("Base",
						layout,
						surface,
						background,
						size,
						center);
				} base;

				struct : ofParameterGroup
				{
					ofParameter<float> width{ "Width", 0.0f, 0.0f, 5.0f };
					ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };

					PARAM_DECLARE("Border", 
						width, 
						color);
				} border;

				struct : ofParameterGroup
				{
					ofParameter<int> type{ "Type", static_cast<int>(Type::Unknown), static_cast<int>(Type::Unknown), static_cast<int>(Type::Sound) };
					ofParameter<float> duration{ "Duration", 0.5f, 0.1f, 5.0f };

					PARAM_DECLARE("Transition", 
						type, 
						duration);
				} transition;

				PARAM_DECLARE("Pop-up", 
					base, 
					border, 
					transition);
			};

			virtual BaseParameters & getParameters() = 0;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}