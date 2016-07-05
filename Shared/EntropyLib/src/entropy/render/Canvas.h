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

			void update();

			void begin();
			void end();

			void draw();

			float getWidth() const;
			float getHeight() const;

			void setWidth(float width);
			void setHeight(float height);

			bool getFillWindow() const;
			void setFillWindow(bool fillWindow);

			shared_ptr<ofxWarp::WarpBase> addWarp(ofxWarp::WarpBase::Type type);
			void removeWarp();

			bool isEditing() const;

			void drawGui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			string getSettingsFilePath();

			bool loadSettings();
			bool saveSettings();

			bool selectClosestControlPoint(const glm::vec2 & pos);

			bool cursorMoved(const glm::vec2 & pos);
			bool cursorDown(const glm::vec2 & pos);
			bool cursorDragged(const glm::vec2 & pos);

			bool keyPressed(ofKeyEventArgs & args);

			void windowResized(ofResizeEventArgs & args);
			
			ofEvent<ofResizeEventArgs> resizeEvent;

			static const int MAX_NUM_WARPS = 8;

		protected:
			void updateSize();

			void updateStitches();

		protected:
			struct WarpParameters
				: ofParameterGroup
			{
				ofxPreset::Parameter<bool> editing{ "Edit Shape", false, false };
				ofxPreset::Parameter<float> brightness{ "Brightness", 1.0f, 0.0f, 1.0f };

				struct : ofParameterGroup
				{
					ofxPreset::Parameter<bool> adaptive{ "Adaptive", true, false };
					ofxPreset::Parameter<bool> linear{ "Linear", false, false };

					PARAM_DECLARE("Mesh", adaptive, linear);
				} mesh;

				struct : ofParameterGroup
				{
					ofxPreset::Parameter<glm::vec3> luminance{ "Luminance", glm::vec3(0.5f), glm::vec3(0.0f), glm::vec3(1.0f) };
					ofxPreset::Parameter<glm::vec3> gamma{ "Gamma", glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(1.0f) };
					ofxPreset::Parameter<float> exponent{ "Exponent", 2.0f, 0.1f, 20.0f };
					ofxPreset::Parameter<float> edgeLeft{ "Edge Left", 0.0f, 0.0f, 1.0f };
					ofxPreset::Parameter<float> edgeRight{ "Edge Right", 0.0f, 0.0f, 1.0f };

					PARAM_DECLARE("Blend", luminance, gamma, exponent, edgeLeft, edgeRight);
				} blend;

				PARAM_DECLARE("Parameters", editing, brightness, mesh, blend);
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
			vector<WarpParameters> warpParameters;
			bool openGuis[MAX_NUM_WARPS];  // Don't use vector<bool> because they're weird: http://en.cppreference.com/w/cpp/container/vector_bool
			
			bool dirtyStitches;
		};
	}
}