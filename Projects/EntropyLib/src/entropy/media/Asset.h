#pragma once

#include "ofParameter.h"
#include "ofxPreset.h"
#include "ofxTLCurves.h"
#include "ofxTLSwitches.h"

#include "entropy/post/ColorEffects.h"
#include "entropy/render/Layout.h"

namespace entropy
{
	namespace media
	{
		static const string MediaTimelinePageName = "Media";
		
		enum class Type
		{
			Unknown,
			Image,
			Movie,
			HPV,
			Sound
		};

		enum class Surface
		{
			Base,
			Overlay
		};

		enum class HorzAlign
		{
			Left,
			Center,
			Right
		};

		enum class VertAlign
		{
			Top,
			Middle,
			Bottom
		};

		enum class SyncMode
		{
			FreePlay,
			Timeline,
			FadeControl,
			LinkedMedia
		};
		
		class Asset
		{
		public:
			Asset(Type type = Type::Unknown);
			virtual ~Asset();

			Type getType() const;
			std::string getTypeName() const;
			bool renderLayout(render::Layout layout);
			Surface getSurface();
			HorzAlign getHorzAlign();
			VertAlign getVertAlign();
			SyncMode getSyncMode();

			float getTotalFade() const;

			std::shared_ptr<Asset> getLinkedMedia() const;
			void setLinkedMedia(std::shared_ptr<Asset> linkedMedia);
			void clearLinkedMedia();

			void refreshTwisterSync();

			bool editing;

			// Base methods
			void init_(int index, int page, std::shared_ptr<ofxTimeline> timeline);
			void clear_();

			void setup_();
			void exit_();
			void resize_(ofResizeEventArgs & args);

			void update_(double dt);
			void draw_();

			void gui_(ofxImGui::Settings & settings);
			
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

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

			virtual uint64_t getCurrentTimeMs() const = 0;
			virtual uint64_t getCurrentFrame() const = 0;

			virtual uint64_t getDurationMs() const = 0;
			virtual uint64_t getDurationFrames() const = 0;
			
			virtual uint64_t getFrameRate() const = 0;

			virtual uint64_t getPlaybackTimeMs(bool wrap = true);
			virtual uint64_t getPlaybackFrame();

			// Parameters
			struct : ofParameterGroup
			{
				ofParameter<string> filePath{ "File Path", "" };

				struct : ofParameterGroup
				{
					ofParameter<ofFloatColor> background{ "Background", ofFloatColor(0.0f, 0.0f) };
					ofParameter<bool> renderBack{ "Render Back", true };
					ofParameter<bool> renderFront{ "Render Front", false };
					ofParameter<int> surface{ "Surface", static_cast<int>(Surface::Overlay), static_cast<int>(Surface::Base), static_cast<int>(Surface::Overlay) };
					ofParameter<float> size{ "Size", 1.0f, 0.0f, 3.0f };
					ofParameter<glm::vec2> anchor{ "Anchor", glm::vec2(0.5f), glm::vec2(0.0f), glm::vec2(1.0f) };
					ofParameter<int> alignHorz{ "Horz Align", static_cast<int>(HorzAlign::Center), static_cast<int>(HorzAlign::Left), static_cast<int>(HorzAlign::Right) };
					ofParameter<int> alignVert{ "Vert Align", static_cast<int>(VertAlign::Middle), static_cast<int>(VertAlign::Top), static_cast<int>(VertAlign::Bottom) };

					PARAM_DECLARE("Base",
						background,
						renderBack, renderFront,
						surface,
						size, anchor,
						alignHorz, alignVert);
				} render;

				entropy::post::ColorParameters color;

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
					ofParameter<bool> useFadeTrack{ "Use Fade Track", true };
					ofParameter<float> fadeTrack{ "Fade Track", 1.0f, 0.0f, 1.0f };
					ofParameter<bool> useFadeTwist{ "Use Fade Twist", false };
					ofParameter<int> fadeKnob{ "Fade Knob", 16, 0, 15 };
					ofParameter<float> fadeTwist{ "Fade Twist", 1.0f, 0.0f, 1.0f };
					ofParameter<bool> useFadeLFO{ "Use Fade LFO", false };
					ofParameter<bool> flipLFO{ "Flip LFO", false };
					ofParameter<float> fadeLFO{ "Fade LFO", 1.0f, 0.0f, 1.0f };
					ofParameter<bool> loop{ "Loop", false };
					ofParameter<int> syncMode{ "Sync Mode", static_cast<int>(SyncMode::Timeline), static_cast<int>(SyncMode::FreePlay), static_cast<int>(SyncMode::LinkedMedia) };
					ofParameter<float> freeSpeed{ "Free Speed", 1.0f, 0.0f, 2.0f };
					ofParameter<bool> useSpeedTwist{ "Use Speed Twist", false };
					ofParameter<int> speedKnob{ "Speed Knob", 16, 0, 15 };

					PARAM_DECLARE("Playback",
						useFadeTrack, fadeTrack,
						useFadeTwist, fadeKnob, fadeTwist,
						useFadeLFO, flipLFO, fadeLFO,
						loop, syncMode,
						freeSpeed, useSpeedTwist, speedKnob);
				} playback;

				PARAM_DECLARE("Media",
					filePath,
					render,
					color,
					border,
					playback);
			} parameters;

		protected:
			virtual bool loadMedia(const std::filesystem::path & filePath) = 0;
			virtual bool isLoaded() const = 0;

			virtual float getContentWidth() const = 0;
			virtual float getContentHeight() const = 0;
			virtual void renderContent() = 0;

			bool shouldPlay();
			virtual bool initFreePlay() = 0;

			Type type;
			int index;
			int page;

			void updateBounds();
			ofRectangle viewport;
			ofRectangle roi;
			bool boundsDirty;

			void updateBorder();
			bool borderDirty;

			string fileName;
			bool wasLoaded;

			float switchMillis;
			float prevFade;
			float lfoVal;

			uint64_t freePlayElapsedLastMs;
			uint64_t freePlayMediaStartMs;
			uint64_t freePlayMediaStartFrame;
			uint64_t freePlayMediaLastMs;
			bool freePlayNeedsInit;

			std::shared_ptr<Asset> linkedMedia;

			// Timeline
			void addSwitchesTrack();
			void removeSwitchesTrack();
			bool addDefaultSwitch();

			void addCurvesTrack();
			void removeCurvesTrack();

			void addLFOTrack();
			void removeLFOTrack();

			// Twister
			void addTwisterFade();
			void removeTwisterFade();

			void addTwisterSpeed();
			void removeTwisterSpeed();

			int twisterFade;
			int twisterSpeed;

			// Per-frame attributes.
			ofRectangle srcBounds;
			ofRectangle dstBounds;
			ofVboMesh borderMesh;

			// Timeline
			std::shared_ptr<ofxTimeline> timeline;
			ofxTLSwitches * switchesTrack;
			ofxTLCurves * curvesTrack;
			ofxTLCurves * lfoTrack;
			bool enabled;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}