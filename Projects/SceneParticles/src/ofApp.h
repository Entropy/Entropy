#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTextureRecorder.h"
#include "ofxTimeline.h"

#include "entropy/Helpers.h"
#include "entropy/particles/ParticleSystem.h"
#include "entropy/particles/Photons.h"
#include "entropy/particles/Environment.h"
#include "entropy/render/PostEffects.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/particles/TextRenderer.h"

class ofApp 
	: public ofBaseApp
{
public:
	void setup();
	void exit();

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

public:
	//bool saveState(const string & path);
	//bool loadState(const string & path);

	bool loadPreset(const string & presetName);
	bool savePreset(const string & presetName);

	void reset();

	static const string kSceneName;
	static const float kHalfDim;
	static const unsigned int kMaxLights;

	ofEasyCam camera;

	nm::ParticleSystem particleSystem;
	nm::Photons photons;
	nm::Environment::Ptr environment;

	ofShader shader;
	ofShader::TransformFeedbackSettings shaderSettings;
	ofBufferObject feedbackBuffer;
	ofVbo feedbackVbo;
	GLuint numPrimitives, numPrimitivesQuery;
	std::vector<ofLight> pointLights;

	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	TextRenderer textRenderer;
	ofFbo fboScene;
	ofFbo fboPost;

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;
	//ofxTLCameraTrack cameraTrack;

	ofxTextureRecorder textureRecorder;

	std::string currPreset;

	struct : ofParameterGroup
	{
		ofParameter<void> resetScene{ "Reset Scene" };
		ofParameter<bool> debugLights{ "Debug Lights", false };

		//ofParameter<string> stateFile;

		struct : ofParameterGroup
		{
			ofParameter<bool> colorsPerType{ "Color per Type", true };
			ofParameter<bool> additiveBlending{ "Additive Blend", true };
			ofParameter<bool> glOneBlending{ "GL_ONE", false };
			ofParameter<bool> drawModels{"Draw models", true};
			ofParameter<bool> drawPhotons{ "Draw Photons", true };
			ofParameter<bool> drawText{ "Draw Text", false };
			ofParameter<float> ambientLight{ "Ambient Light", 0.001, 0, 0.02 };
			ofParameter<float> attenuation{ "Attenuation", 0.01, 0.0000001, 0.05 };
			ofParameter<float> lightStrength{ "Light Strength", 1, 0, 1 };
			ofParameter<float> fov{ "Fov", 60, 1, 120 };
			ofParameter<float> rotationRadius{"Rotation radius", 1, 0.5, 5, ofParameterScale::Logarithmic};
			ofParameter<float> rotationSpeed{"Rotation speed", 1, 0.1, 100, ofParameterScale::Logarithmic};

			PARAM_DECLARE("Rendering",
				colorsPerType,
				additiveBlending,
				glOneBlending,
				drawModels,
				drawPhotons,
				drawText,
				ambientLight,
				attenuation,
				fov,
			  rotationRadius,
			  rotationSpeed);
		} rendering;

		struct : ofParameterGroup
		{
			ofParameter<int>  fps{ "fps", 60, 20, 1200, ofParameterScale::Logarithmic};
			ofParameter<bool> systemClock{"system clock", false};
			ofParameter<bool> recordSequence{ "Record Sequence", false };
			ofParameter<bool> recordVideo{ "Record Video", false };

			PARAM_DECLARE("Recording",
				fps,
				systemClock,
				recordSequence,
				recordVideo);
		} recording;

		PARAM_DECLARE("Scene",
			resetScene,
			debugLights,
			//stateFile,
			rendering,
			recording);
	} parameters;

	double now = 0;
	double dt = 0;
	double orbitAngle = 0;
};
