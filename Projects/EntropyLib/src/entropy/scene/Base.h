#pragma once

#include "ofMain.h"
#include "ofxPreset.h"
#include "ofxTimeline.h"

#include "entropy/geom/Box.h"
#include "entropy/popup/Image.h"
#include "entropy/popup/Video.h"
#include "entropy/render/Layout.h"
#include "entropy/render/PostEffects.h"
#include "entropy/util/Mapping.h"

namespace entropy
{
	namespace scene
	{
		static const string kPresetDefaultName = "_default";
		
		class Base
		{
		public:
			virtual string getName() const = 0;

			Base();
			virtual ~Base();

			// Base methods
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

			// Override methods
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

			// No post-processing by default, return true to override!
			virtual bool postProcessBack(const ofTexture & srcTexture, const ofFbo & dstFbo) { return false; }
			virtual bool postProcessFront(const ofTexture & srcTexture, const ofFbo & dstFbo) { return false; };

			virtual void gui(ofxPreset::Gui::Settings & settings) {}

			virtual void serialize(nlohmann::json & json) {}
			virtual void deserialize(const nlohmann::json & json) {}

			// Resources
			string getAssetsPath(const string & file = "");
			string getDataPath(const string & file = "");
			string getPresetPath(const string & preset = "");
			string getCurrentPresetPath(const string & file = "");

			const vector<string> & getPresets() const;
			const string & getCurrentPresetName() const;

			bool loadPreset(const string & presetName);
			bool savePreset(const string & presetName);

			ofEvent<string> presetLoadedEvent;

			// Timeline
			void drawTimeline(ofxPreset::Gui::Settings & settings);
			int getCurrentTimelineFrame();

			// Camera
			ofEasyCam & getCameraBack();
			ofEasyCam & getCameraFront();

			void setCameraControlArea(render::Layout layout, const ofRectangle & controlArea);

			void setCameraLocked(bool cameraLocked);
			void toggleCameraLocked();
			bool isCameraLocked() const;

            void addCameraKeyframe(render::Layout layout);

            // Post Effects.
            render::PostParameters & getPostParameters(render::Layout layout);

			// Export
			void beginExport();
			void endExport();

		protected:
			// Camera
			virtual void resetCamera(render::Layout layout);

			std::map<render::Layout, ofEasyCam> cameras;

			// Box
			std::map<render::Layout, geom::Box> boxes;

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
						ofParameter<bool> relativeYAxis{ "Relative Y Axis", false };
						ofParameter<bool> attachFrontToBack{ "Attach Front to Back", true };
						ofParameter<int> mouseEnabled{ "Mouse Enabled", static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Front) };
						ofParameter<float> backClipNear{ "Clip Near Back", 0.0001f, 0.0001f, 1000.0f };
						ofParameter<float> backClipFar{ "Clip Far Back", 1000.0f, 0.0001f, 1000.0f };
						ofParameter<float> frontClipNear{ "Clip Near Front", 0.0001f, 0.0001f, 1000.0f };
						ofParameter<float> frontClipFar{ "Clip Far Front", 1000.0f, 0.0001f, 1000.0f };

						PARAM_DECLARE("Camera", relativeYAxis, attachFrontToBack, mouseEnabled, backClipNear, backClipFar, frontClipNear, frontClipFar);
					} camera;

					PARAM_DECLARE("Base", background, camera);
				} base;

				PARAM_DECLARE("Parameters", base);
			};

            virtual BaseParameters & getParameters() = 0;

            std::map<render::Layout, render::PostParameters> postEffects;

			std::vector<ofEventListener> parameterListeners;

			// Timeline
			ofxTimeline timeline;
			map<render::Layout, ofxTLCameraTrack *> cameraTracks;
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
