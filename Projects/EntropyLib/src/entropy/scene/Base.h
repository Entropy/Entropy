#pragma once

#include "ofFileUtils.h"
#include "ofxImGui.h"
#include "ofxOsc.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "entropy/geom/Box.h"
#include "entropy/media/Base.h"
#include "entropy/media/Link.h"
#include "entropy/render/Layout.h"
#include "entropy/render/PostEffects.h"
#include "entropy/util/Mapping.h"
#include "entropy/world/Camera.h"

namespace entropy
{
	namespace scene
	{
		static const string kPresetDefaultName = "_autosave";
		
		class Base
		{
		public:
			virtual string getName() const = 0;

			Base();
			virtual ~Base();

			// Base methods
			void init_();
			void clear_();

			void setup_();
			void exit_();

			void resize_(render::Layout layout, ofResizeEventArgs & args);

			void update_(double dt);

			void draw_(render::Layout layout);

			void gui_(ofxImGui::Settings & settings);

			void serialize_(nlohmann::json & json);
			void deserialize_(const nlohmann::json & json);

			// State
			bool isInitialized() const;
			bool isReady() const;

			// Resources
			std::filesystem::path getAssetsPath(const string & file = "");
			std::filesystem::path getDataPath(const string & file = "");
			std::filesystem::path getPresetPath(const string & preset = "");
			std::filesystem::path getCurrentPresetPath(const string & file = "");

			const vector<string> & getPresets() const;
			const string & getCurrentPresetName() const;

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			ofEvent<string> presetCuedEvent;
			ofEvent<string> presetLoadedEvent;
			ofEvent<string> presetSavedEvent;

			void setShowtime();
			
			// Timeline
			void drawTimeline(ofxImGui::Settings & settings);
			int getCurrentTimelineFrame();
			bool goToNextTimelineFlag();

			// Post Effects.
			render::PostParameters & getPostParameters(render::Layout layout);

			// Export
			void beginExport();
			void endExport();

		protected:
			// Override methods
			virtual void init() {}
			virtual void clear() {}

			virtual void setup() {}
			virtual void exit() {}

			virtual void resizeBack(ofResizeEventArgs & args) {}
			virtual void resizeFront(ofResizeEventArgs & args) {}

			virtual void update(double dt) {}

			virtual void drawBack() {}
			virtual void drawFront() {}

			virtual void gui(ofxImGui::Settings & settings) {}

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

			virtual void timelineBangFired(ofxTLBangEventArgs & args) {}
			virtual void messageReceived(ofxOscMessage & message) {}

			// State
			bool initialized;
			bool ready;

			// Resources
			void populatePresets();

			std::filesystem::path assetsPath;
			std::filesystem::path dataPath;
			string currPreset;
			vector<string> presets;

			// Helpers
			void loadTextureImage(const std::string & filePath, ofTexture & texture);

			// Parameters
			virtual ofParameterGroup & getParameters() = 0;

			std::vector<ofEventListener> parameterListeners;

			// Timeline
			void timelineBangFired_(ofxTLBangEventArgs & args);

			std::shared_ptr<ofxTimeline> timeline;
			ofxTLFlags * cuesTrack;
			ofxTLFlags * messagesTrack;

			// Mappings
			void populateMappings(const ofParameterGroup & group, const std::string & timelinePageName = util::kMappingTimelinePageName);
			void refreshMappings();
			void clearMappings();

			std::map<std::string, std::vector<std::shared_ptr<util::AbstractMapping>>> mappings;

			// Media
			std::shared_ptr<media::Base> addMedia(media::Type type, int page);
			void removeMedia();

			int mediaPage;
			std::vector<std::shared_ptr<media::Base>> medias;

			// Links
			std::shared_ptr<media::Link> addLink(size_t idxA, size_t idxB, bool direction);
			void removeLink();

			std::vector<std::shared_ptr<media::Link>> links;

			int linkMediaA;
			int linkMediaB;
			bool linkDirection;

			// Messenger
			void messageReceived_(ofxOscMessage & message);
		};
	}
}
