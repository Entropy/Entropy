#pragma once

#include "ofFileUtils.h"
#ifdef OFX_PARAMETER_TWISTER
#include "ofxParameterTwister.h"
#endif

#include "entropy/render/Canvas.h"
#include "entropy/scene/Playlist.h"
#include "entropy/util/Messenger.h"
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

			std::shared_ptr<render::Canvas> getCanvas(render::Layout layout);
			std::shared_ptr<util::Messenger> getMessenger() const;
			std::shared_ptr<scene::Playlist> getPlaylist() const;
#ifdef OFX_PARAMETER_TWISTER
			std::shared_ptr<pal::Kontrol::ofxParameterTwister> getTwister() const;
#endif

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
			const std::filesystem::path & getDataPath();
			const std::filesystem::path & getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			void processCanvas(render::Layout layout, bool renderEnabled);

			void drawGui(ofxImGui::Settings & settings);

			void applyConfiguration();
			void updatePreviews();
			void updateWarpOutline(render::Layout layout);
			void updateCanvasOutline(render::Layout layout);

		protected:
			std::map<render::Layout, std::shared_ptr<render::Canvas>> canvas;
			std::shared_ptr<util::Messenger> messenger;
			std::shared_ptr<scene::Playlist> playlist;
#ifdef OFX_PARAMETER_TWISTER
			std::shared_ptr<pal::Kontrol::ofxParameterTwister> twister;
#endif

			struct PreviewData
			{
				ofRectangle warpBounds;
				ofRectangle canvasBounds;
				ofVboMesh warpOutline;
				ofVboMesh canvasOutline;
			};
			std::map<render::Layout, ofRectangle> screenBounds;
			std::map<render::Layout, PreviewData> previewData;
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
						ofParameter<bool> backWarp{ "Back Warp", false, };
						ofParameter<bool> backCanvas{ "Back Canvas", false, };
						ofParameter<bool> frontWarp{ "Front Warp", false, };
						ofParameter<bool> frontCanvas{ "Front Canvas", false, };
						ofParameter<float> scale{ "Scale", 0.5f, 0.1f, 1.0f };

						PARAM_DECLARE("Preview", 
							backWarp, backCanvas,
							frontWarp, frontCanvas,
							scale);
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
	
	//--------------------------------------------------------------
	inline shared_ptr<util::Messenger> GetMessenger()
	{
		return GetApp()->getMessenger();
	}
}
