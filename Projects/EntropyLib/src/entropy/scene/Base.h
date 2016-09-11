#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "entropy/geom/Box.h"
#include "entropy/popup/Image.h"
#include "entropy/popup/Sound.h"
#include "entropy/popup/Video.h"
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

			void drawBase_(render::Layout layout);
			void drawWorld_(render::Layout layout);
			void drawOverlay_(render::Layout layout);

			void gui_(ofxPreset::Gui::Settings & settings);

			void serialize_(nlohmann::json & json);
			void deserialize_(const nlohmann::json & json);

			// State
			bool isInitialized() const;
			bool isReady() const;

			// No post-processing by default, return true to override!
			virtual bool postProcessBack(const ofTexture & srcTexture, const ofFbo & dstFbo) { return false; }
			virtual bool postProcessFront(const ofTexture & srcTexture, const ofFbo & dstFbo) { return false; };

			// Resources
			string getAssetsPath(const string & file = "");
			string getDataPath(const string & file = "");
			string getPresetPath(const string & preset = "");
			string getCurrentPresetPath(const string & file = "");

			const vector<string> & getPresets() const;
			const string & getCurrentPresetName() const;

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			ofEvent<string> presetCuedEvent;
			ofEvent<string> presetLoadedEvent;
			ofEvent<string> presetSavedEvent;

			void setShowtime();
			
			// Timeline
			void drawTimeline(ofxPreset::Gui::Settings & settings);
			int getCurrentTimelineFrame();
			bool goToNextTimelineFlag();

			// Camera
			std::shared_ptr<world::Camera> getCamera(render::Layout layout);

			void setCameraControlArea(render::Layout layout, const ofRectangle & controlArea);

			void setCameraLocked(bool cameraLocked);
			void toggleCameraLocked();
			bool isCameraLocked(render::Layout layout) const;

			void addCameraKeyframe(render::Layout layout);

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

			virtual void drawBackBase() {}
			virtual void drawBackWorld() {}
			virtual void drawBackOverlay() {}

			virtual void drawFrontBase() {}
			virtual void drawFrontWorld() {}
			virtual void drawFrontOverlay() {}

			virtual void gui(ofxPreset::Gui::Settings & settings) {}

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

			virtual void timelineBangFired(ofxTLBangEventArgs & args) {}

			// State
			bool initialized;
			bool ready;

			// Camera
			std::map<render::Layout, std::shared_ptr<world::Camera>> cameras;

			// Box
			std::map<render::Layout, geom::Box> boxes;

			// Post Effects
			std::map<render::Layout, render::PostParameters> postEffects;

			// Resources
			void populatePresets();

			string assetsPath;
			string dataPath;
			string currPreset;
			vector<string> presets;

			// Parameters
			virtual ofParameterGroup & getParameters() = 0;

			std::vector<ofEventListener> parameterListeners;

			// Timeline
			void timelineBangFired_(ofxTLBangEventArgs & args);

			std::shared_ptr<ofxTimeline> timeline;
			ofxTLFlags * cuesTrack;

			// Mappings
			void populateMappings(const ofParameterGroup & group, const std::string & timelinePageName = util::kMappingTimelinePageName);
			void refreshMappings();
			void clearMappings();

			std::map<std::string, std::vector<std::shared_ptr<util::AbstractMapping>>> mappings;

			// Popups
			std::shared_ptr<popup::Base> addPopUp(popup::Type type);
			void removePopUp();

			std::vector<std::shared_ptr<popup::Base>> popUps;
		};
	}
}
