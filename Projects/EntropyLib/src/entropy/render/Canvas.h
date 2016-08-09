#pragma once

#include "ofMain.h"

#include "ofxPreset.h"
#include "ofxTextureRecorder.h"
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

			void beginDraw();
			void endDraw();

			void render(bool postProcessing);

			const ofTexture & getDrawTexture() const;
			const ofFbo & getPostFbo() const;

			float getWidth() const;
			float getHeight() const;
			const ofRectangle & getViewport() const;

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
				ofParameter<bool> editing{ "Edit Shape", false };
				ofParameter<float> brightness{ "Brightness", 1.0f, 0.0f, 1.0f };

				struct : ofParameterGroup
				{
					ofParameter<bool> adaptive{ "Adaptive", true };
					ofParameter<bool> linear{ "Linear", false };

					PARAM_DECLARE("Mesh", adaptive, linear);
				} mesh;

				struct : ofParameterGroup
				{
					ofParameter<glm::vec3> luminance{ "Luminance", glm::vec3(0.5f), glm::vec3(0.0f), glm::vec3(1.0f) };
					ofParameter<glm::vec3> gamma{ "Gamma", glm::vec3(1.0f), glm::vec3(0.0f), glm::vec3(1.0f) };
					ofParameter<float> exponent{ "Exponent", 2.0f, 0.1f, 20.0f };
					ofParameter<float> edgeLeft{ "Edge Left", 0.0f, 0.0f, 1.0f };
					ofParameter<float> edgeRight{ "Edge Right", 0.0f, 0.0f, 1.0f };

					PARAM_DECLARE("Blend", luminance, gamma, exponent, edgeLeft, edgeRight);
				} blend;

				PARAM_DECLARE("Parameters", editing, brightness, mesh, blend);
			};

			struct : ofParameterGroup
			{
				ofParameter<bool> fillWindow{ "Fill Window", false };

				PARAM_DECLARE("Parameters", fillWindow);
			} parameters;

			ofRectangle viewport;

			ofFbo fboDraw;
			ofFbo fboPost;
			ofFbo::Settings fboSettings;

			bool exportFrames;
			ofxTextureRecorder textureRecorder;

			vector<shared_ptr<ofxWarp::WarpBase>> warps;
			size_t focusedIndex;

			vector<ofRectangle> srcAreas;
			vector<WarpParameters> warpParameters;
			bool openGuis[MAX_NUM_WARPS];  // Don't use vector<bool> because they're weird: http://en.cppreference.com/w/cpp/container/vector_bool
			
			bool dirtyStitches;
		};
	}
}