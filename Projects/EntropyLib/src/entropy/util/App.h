#pragma once

#include "entropy/render/Canvas.h"
#include "entropy/scene/Playlist.h"
#include "entropy/util/Singleton.h"

namespace entropy
{
	namespace util
	{
		class App_
		{
		public:
			App_();
			~App_();

			shared_ptr<render::Canvas> getCanvas(render::Layout layout);
			shared_ptr<scene::Playlist> getPlaylist() const;

			const ofRectangle & getScreenBounds(render::Layout layout);
			const ofRectangle & getBoundsControl() const;

			bool isMouseOverGui() const;
			bool isControlsVisible() const;

		protected:
			void onUpdate(ofEventArgs & args);
			void onDraw(ofEventArgs & args);

			void onMouseMoved(ofMouseEventArgs & args);
			void onMousePressed(ofMouseEventArgs & args);
			void onMouseDragged(ofMouseEventArgs & args);
			void onMouseReleased(ofMouseEventArgs & args);

			void onKeyPressed(ofKeyEventArgs & args);
			void onKeyReleased(ofKeyEventArgs & args);

			void onCanvasBackResized(ofResizeEventArgs & args);
			void onCanvasFrontResized(ofResizeEventArgs & args);
			void onWindowResized(ofResizeEventArgs & args);

			std::vector<ofEventListener> parameterListeners;

		protected:
			const string & getDataPath();
			const string & getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			void processCanvas(render::Layout layout, bool renderEnabled);

			void drawGui(ofxPreset::Gui::Settings & settings);

			void applyConfiguration();
			void updatePreviews();

		protected:
			std::map<render::Layout, shared_ptr<render::Canvas>> canvas;
			shared_ptr<scene::Playlist> playlist;

			std::map<render::Layout, ofRectangle> screenBounds;
			std::map<render::Layout, ofRectangle> previewBounds;
			ofRectangle boundsControl;

			ofxImGui imGui;
			ofxPreset::Gui::Settings guiSettings;

			struct : ofParameterGroup
			{
				ofParameter<ofFloatColor> background{ "Background", ofFloatColor::black };
			
				struct : ofParameterGroup
				{
					ofParameter<bool> enabled{ "Enabled", true };
					ofParameter<int> screenWidth{ "Screen Width", 1920, 1280, 1920 };
					ofParameter<int> screenHeight{ "Screen Height", 1080, 720, 1080 };

					struct : ofParameterGroup
					{
						ofParameter<bool> backEnabled{ "Back Enabled", true };
						ofParameter<bool> frontEnabled{ "Front Enabled", true };
						ofParameter<float> scale{ "Scale", 0.5f, 0.1f, 1.0f };

						PARAM_DECLARE("Preview", backEnabled, frontEnabled, scale);
					} preview;
					
					PARAM_DECLARE("Control Screen", enabled, screenWidth, screenHeight, preview);
				} controlScreen;

				struct : ofParameterGroup
				{
					ofParameter<bool> enabled{ "Enabled", true };
					ofParameter<int> screenWidth{ "Screen Width", 1920, 1280, 1920 };
					ofParameter<int> screenHeight{ "Screen Height", 1080, 720, 1080 };
					ofParameter<int> numRows{ "Num Rows", 1, 1, 3 };
					ofParameter<int> numCols{ "Num Cols", 1, 1, 3 };

					PARAM_DECLARE("Back Screen", enabled, screenWidth, screenHeight, numRows, numCols);
				} backScreen;

				struct : ofParameterGroup
				{
					ofParameter<bool> enabled{ "Enabled", true };
					ofParameter<int> screenWidth{ "Screen Width", 1920, 1280, 1920 };
					ofParameter<int> screenHeight{ "Screen Height", 1080, 720, 1080 };
					ofParameter<int> numRows{ "Num Rows", 1, 1, 3 };
					ofParameter<int> numCols{ "Num Cols", 1, 1, 3 };

					PARAM_DECLARE("Front Screen", enabled, screenWidth, screenHeight, numRows, numCols);
				} frontScreen;

				PARAM_DECLARE("App", background, controlScreen, backScreen, frontScreen);
			} parameters;

			bool controlsVisible;
		};

		typedef util::Singleton<App_> App;
	}
}