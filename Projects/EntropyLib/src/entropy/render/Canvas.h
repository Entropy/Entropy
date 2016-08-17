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
			Canvas(const string & name);
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

			const string & getDataPath();
			const string & getSettingsFilePath();
			string getShaderPath(const string & shaderFile = "");
			
			bool loadSettings();
			bool saveSettings();

			bool selectClosestControlPoint(const glm::vec2 & pos);

			bool cursorMoved(const glm::vec2 & pos);
			bool cursorDown(const glm::vec2 & pos);
			bool cursorDragged(const glm::vec2 & pos);

			bool keyPressed(ofKeyEventArgs & args);

			void screenResized(ofResizeEventArgs & args);
			
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

				PARAM_DECLARE("Warp", editing, brightness, mesh, blend);
			};

			struct : ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<bool> enabled{ "Enabled", true };
					ofParameter<int> numPasses{ "Num Passes", 1, 1, 10 };
					ofParameter<float> brightnessThreshold{ "Brightness Threshold", 1.0f, 0.01f, 3.0f };
					ofParameter<float> sigma{ "Sigma", 0.9f, 0.5f, 18.0f };
					ofParameter<bool> debugBlur{ "Debug blur", false };

					PARAM_DECLARE("Bloom", enabled, numPasses, brightnessThreshold, sigma, debugBlur);
				} bloom;

				struct : ofParameterGroup
				{
					ofParameter<float> exposure{ "Exposure", 4.0f, 0.1f, 10.0f };
					ofParameter<float> gamma{ "Gamma", 2.2f, 0.01f, 10.0f };
					ofParameter<int> tonemapping{ "Tonemapping", 6, 0, 6 };
					ofParameter<float> contrast{ "Contrast", 1.0f, 0.5f, 1.5f };
					ofParameter<float> brightness{ "Brightness", 0.0f, -1.0f, 1.0f };

					PARAM_DECLARE("Color", exposure, gamma, tonemapping, contrast, brightness);
				} color;

				ofParameter<bool> fillWindow{ "Fill Window", false };

				PARAM_DECLARE("Canvas", bloom, color, fillWindow);
			} parameters;

			ofRectangle viewport;
			
			ofFbo fboDraw;
			ofFbo fboPost;
			ofFbo fboTemp[2];
			ofFbo::Settings fboSettings;

			GLuint defaultVao;

			ofShader brightnessThresholdShader;
			ofShader blurHorzShader;
			ofShader blurVertShader;
			ofShader colorCorrectShader;

			bool exportFrames;
			ofxTextureRecorder textureRecorder;

			float screenWidth;
			float screenHeight;
			
			vector<shared_ptr<ofxWarp::WarpBase>> warps;
			size_t focusedIndex;

			vector<ofRectangle> srcAreas;
			vector<WarpParameters> warpParameters;
			bool openGuis[MAX_NUM_WARPS];  // Don't use vector<bool> because they're weird: http://en.cppreference.com/w/cpp/container/vector_bool
			
			bool dirtyStitches;
			ofVboMesh fullQuad;
		};
	}
}