#pragma once

#include "entropy/render/Canvas.h"
#include "entropy/scene/Playlist.h"
#include "entropy/util/Messenger.h"
#include "entropy/util/Singleton.h"

namespace entropy
{
	namespace util
	{
		enum class Preview
		{
			None,
			Warp,
			Full
		};
		
		class App_
		{
		public:
			App_();
			~App_();

			shared_ptr<render::Canvas> getCanvas(render::Layout layout);
			shared_ptr<util::Messenger> getMessenger() const;
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

			void drawGui(ofxImGui::Settings & settings);

			void applyConfiguration();
			void updatePreviews();
			void updateOutline(render::Layout layout);

		protected:
			std::map<render::Layout, shared_ptr<render::Canvas>> canvas;
			shared_ptr<util::Messenger> messenger;
			shared_ptr<scene::Playlist> playlist;

			std::map<render::Layout, ofRectangle> screenBounds;
			std::map<render::Layout, ofRectangle> previewBounds;
			std::map<render::Layout, ofVboMesh> previewOutlines;
			ofRectangle boundsControl;

			ofxImGui::Gui imGui;
			ofxImGui::Settings guiSettings;

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
						ofParameter<int> modeBack{ "Back", static_cast<int>(util::Preview::Full), static_cast<int>(util::Preview::None), static_cast<int>(util::Preview::Full) };
						ofParameter<int> modeFront{ "Front", static_cast<int>(util::Preview::Full), static_cast<int>(util::Preview::None), static_cast<int>(util::Preview::Full) };
						ofParameter<float> scale{ "Scale", 0.5f, 0.1f, 1.0f };

						PARAM_DECLARE("Preview", modeBack, modeFront, scale);
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


	//--------------------------------------------------------------
	inline util::App_ * GetApp()
	{
		return util::App::X();
	}

	//--------------------------------------------------------------
	inline shared_ptr<render::Canvas> GetCanvas(render::Layout layout)
	{
		return GetApp()->getCanvas(layout);
	}

	//--------------------------------------------------------------
	inline float GetCanvasWidth(render::Layout layout)
	{
		return GetCanvas(layout)->getWidth();
	}

	//--------------------------------------------------------------
	inline float GetCanvasHeight(render::Layout layout)
	{
		return GetCanvas(layout)->getHeight();
	}

	//--------------------------------------------------------------
	inline const ofRectangle & GetCanvasViewport(render::Layout layout)
	{
		return GetCanvas(layout)->getViewport();
	}

	//--------------------------------------------------------------
	inline shared_ptr<scene::Playlist> GetPlaylist()
	{
		return GetApp()->getPlaylist();
	}

	//--------------------------------------------------------------
	inline const world::Camera::Settings & GetSavedCameraSettings(render::Layout layout)
	{
		return GetPlaylist()->getCameraSettings(layout);
	}

	//--------------------------------------------------------------
	inline shared_ptr<scene::Base> GetCurrentScene()
	{
		return GetPlaylist()->getCurrentScene();
	}
}
