#pragma once

#include "ofMain.h"

#include "ofxPreset.h"
#include "ofxWarp.h"

namespace entropy
{
	namespace render
	{
		class Canvas
		{
		public:
			Canvas();
			~Canvas();

			void begin();
			void end();

			void draw();

			float getWidth() const;
			float getHeight() const;

			void setWidth(float width);
			void setHeight(float height);

			bool getFillWindow() const;
			void setFillWindow(bool fillWindow);

			void addWarp(ofxWarp::WarpBase::Type type);
			void removeWarp();

			void updateStitches();

			bool isEditing() const;

			void drawGui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			string getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			bool selectClosestControlPoint(const ofVec2f & pos);

			bool cursorMoved(ofVec2f & pos);
			bool cursorDown(ofVec2f & pos);
			bool cursorDragged(ofVec2f & pos);

			bool keyPressed(ofKeyEventArgs & args);

			void windowResized(ofResizeEventArgs & args);
			
			ofEvent<ofResizeEventArgs> resizeEvent;

			static const int MAX_NUM_WARPS = 8;

		protected:
			struct WarpParameters
				: ofParameterGroup
			{
				ofxPreset::Parameter<bool> editing{ "Edit Shape", false, true };
				ofxPreset::Parameter<float> brightness{ "Brightness", 1.0f, 0.0f, 1.0f };
				ofxPreset::Parameter<bool> adaptive{ "Adaptive", true, true };
				ofxPreset::Parameter<bool> linear{ "Linear", false, true };

				PARAM_DECLARE("Parameters", editing, brightness, adaptive, linear);
			};

			struct : ofParameterGroup
			{
				ofxPreset::Parameter<bool> fillWindow{ "Fill Window", false, true };

				PARAM_DECLARE("Parameters", fillWindow);
			} parameters;

			ofFbo fbo;
			ofFbo::Settings fboSettings;

			vector<shared_ptr<ofxWarp::WarpBase>> warps;
			size_t focusedIndex;

			vector<ofRectangle> srcAreas;
			vector<float> overlaps;
			vector<WarpParameters> warpParameters;
			bool openGuis[MAX_NUM_WARPS];  // Don't use vector<bool> because they're weird: http://en.cppreference.com/w/cpp/container/vector_bool
		};
	}
}