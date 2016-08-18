#pragma once

#include "ofMain.h"

#include "entropy/render/Canvas.h"
#include "entropy/scene/Manager.h"
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

			shared_ptr<entropy::render::Canvas> getCanvasBack() const;
			shared_ptr<entropy::render::Canvas> getCanvasFront() const;
			shared_ptr<entropy::scene::Manager> getSceneManager() const;

			const ofRectangle & getBoundsControl() const;
			const ofRectangle & getBoundsBack() const;
			const ofRectangle & getBoundsFront() const;

			bool isMouseOverGui() const;
			bool isOverlayVisible() const;

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

		protected:
			const string & getDataPath();
			const string & getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			void applyConfiguration();

		protected:
			shared_ptr<entropy::render::Canvas> canvasBack;
			shared_ptr<entropy::render::Canvas> canvasFront;
			shared_ptr<entropy::scene::Manager> sceneManager;

			ofRectangle boundsControl;
			ofRectangle boundsBack;
			ofRectangle boundsFront;

			ofxImGui imGui;
			ofxPreset::Gui::Settings guiSettings;

			struct ScreenParameters
				: ofParameterGroup
			{
				ofParameter<bool> enabled{ "Enabled", true };
				ofParameter<int> screenWidth{ "Screen Width", 1920, 1280, 1920 };
				ofParameter<int> screenHeight{ "Screen Height", 1080, 720, 1080 };
				ofParameter<int> numRows{ "Num Rows", 1, 1, 3 };
				ofParameter<int> numCols{ "Num Cols", 1, 1, 3 };

				PARAM_DECLARE("Screen", enabled, screenWidth, screenHeight, numRows, numCols);
			};

			ScreenParameters controlScreenParameters;
			ScreenParameters backScreenParameters;
			ScreenParameters frontScreenParameters;

			struct : ofParameterGroup
			{
				ofParameter<ofFloatColor> background{ "Background", ofFloatColor::black };
			
				PARAM_DECLARE("App", background);
			} parameters;

			bool overlayVisible;
		};

		typedef entropy::util::Singleton<App_> App;
	}
}