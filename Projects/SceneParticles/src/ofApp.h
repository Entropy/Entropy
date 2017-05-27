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

	ofCamera camera;
	ofCamera cameraViewport;
	ofEasyCam easyCam;

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
			ofParameter<float> rotationRadius{"Rotation radius", 50, 5, 200, ofParameterScale::Logarithmic};
			ofParameter<float> rotationSpeed{"Rotation speed", 1, 0.1, 100, ofParameterScale::Logarithmic};
			ofParameter<float> travelMaxSpeed{"Travel max speed", 0.1, 0.001, 2, ofParameterScale::Logarithmic};
			ofParameter<float> minTimeBetweenTravels{"min time between travels", 5, 0, 20, ofParameterScale::Logarithmic};
			ofParameter<bool> doCameraTracking{"camera tracking", false};
			ofParameter<bool> cutToInteraction{"cut to interaction", false};
			ofParameter<bool> useEasyCam{"use easy cam", false};
			ofParameter<void> addCluster{"add cluster"};
			ofParameter<float> scaleFactor{"scale factor", 0.99, 0.7, 0.999};
			ofParameter<float> trailsAlpha{"trailsAlpha", 1, 0, 1};
			ofParameter<float> particlesAlpha{"particlesAlpha", 1, 0, 1};


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
				rotationSpeed,
				travelMaxSpeed,
				minTimeBetweenTravels,
				doCameraTracking,
				cutToInteraction,
				useEasyCam,
				addCluster,
				scaleFactor,
				trailsAlpha,
				particlesAlpha);
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

		struct : ofParameterGroup
		{
			ofParameter<int>  electrons{ "electrons", 300, 0, 5000};
			ofParameter<int>  positrons{ "positrons", 300, 0, 5000};
			ofParameter<int>  anti_up_quarks{ "anti up quarks", 300, 0, 5000};
			ofParameter<int>  up_quarks{ "up quarks", 300, 0, 5000};
			ofParameter<int>  anti_down_quarks{ "anti down quarks", 300, 0, 5000};
			ofParameter<int>  down_quarks{ "down quarks", 300, 0, 5000};
			ofParameter<int>  neutrons{ "neutrons", 0, 0, 5000};
			ofParameter<int>  protons{ "protons", 0, 0, 5000};
			ofParameter<void> resetScene{ "Reset Scene" };

			PARAM_DECLARE("Reset",
			  electrons,
			  positrons,
			  anti_up_quarks,
			  up_quarks,
			  anti_down_quarks,
			  down_quarks,
			  neutrons,
			  protons,
			  resetScene);
		} reset;

		PARAM_DECLARE("Scene",
			debugLights,
			//stateFile,
			rendering,
			recording,
			reset);
	} parameters;

	std::array<ofParameter<int>, 8> particleTypesInitialNumbers{{
		parameters.reset.electrons,
		parameters.reset.positrons,
		parameters.reset.anti_up_quarks,
		parameters.reset.up_quarks,
		parameters.reset.anti_down_quarks,
		parameters.reset.down_quarks,
		parameters.reset.neutrons,
		parameters.reset.protons,					\
	}};


	double now = 0;
	double dt = 0;
	double orbitAngle = 0;

	std::pair<size_t, size_t> lookAt{0,0};
	std::pair<nm::Particle*, nm::Particle*> currentLookAtParticles{nullptr,nullptr};
	glm::vec3 lookAtPos, prevLookAt, lerpedLookAt, prevCameraPosition;
	bool arrived = true;
	double timeConnectionLost = 0;
	double timeRenewLookAt = 0;
	float travelDistance = 0;
	std::vector<nm::Photon> photonsAlive;
	float rotationDirection = 1;
	float rotationSpeed = 0;



	struct Cluster{
		float scale = 1;
		glm::vec3 origin;
		double startTime;
		float startScale = 1;
		glm::quat rotation;
		float alpha = 1;
	};

	std::vector<Cluster> clusters{};

	nm::Environment::State prevState;

//	float annihilationPct = 0;
//	float pct = 0;
//	ofPolyline cameraPath;
//	std::deque<glm::vec3> currentPath;
//	float traveledLength = 0;
//	float travelSpeed = 0;
};
