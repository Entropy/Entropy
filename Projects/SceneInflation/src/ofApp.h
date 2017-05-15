#pragma once

#include "ofxEasing.h"
#include "ofMain.h"
#include "entropy/inflation/NoiseField.h"
#include "entropy/inflation/GPUMarchingCubes.h"
#include "entropy/inflation/TransitionParticles.h"
#include "Helpers.h"
#include "WireframeFillRenderer.h"
#include "ofxGui.h"
#include "ofxTextureRecorder.h"
#include "PostEffects.h"
#include "ofxTimeline.h"

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
	void mousePressed(ofMouseEventArgs & mouse);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	entropy::inflation::GPUMarchingCubes gpuMarchingCubes;
	TransitionParticles transitionParticles;
	ofBufferObject transitionParticlesPosition;
	ofShader clearParticlesVel;

	entropy::render::WireframeFillRenderer renderer;

	// Noise Field
	entropy::inflation::NoiseField noiseField;

	uint64_t timeToSetIso;
	uint64_t timeToUpdate;

	//ofVbo box;

	enum State{
		PreBigBang,
		//PreBigBangWobble,
		BigBang,
		Expansion,
		ExpansionTransition,
		ParticlesTransition,
	};

	double now = 0.0;
	double t_bigbang = 0.0;
	double t_from_bigbang = 0.0;
	double t_expansion = 0.0;
	double t_from_expansion = 0.0;
	double t_transition = 0.0;
	double t_from_particles = 0.0;


	struct Cluster{
		float scale = 1;
		glm::vec3 origin;
		double startTime;
		float startScale = 1;
	};
	std::vector<Cluster> clusters{Cluster()};
	float cameraDistanceBeforeBB;
	bool octavesResetDuringTransition=false;
	bool firstCycle;
	bool needsParticlesUpdate;

	std::array<float,4> targetWavelengths;

	struct : ofParameterGroup
	{
		// original orange-ish looks like hot stones
	//	const std::array<ofFloatColor,4> preBigbangColors{{
	//		ofColor{117.f,118.f,118.f},
	//		ofColor{200.,200.,200.},
	//		ofColor(240.,127.,19.),
	//		ofColor(128.,9.,9.),
	//	}};

		// light blue - looks like ice
	//	const std::array<ofFloatColor,4> preBigbangColors{{
	//		ofColor{82.f,195.f,253.f},
	//		ofColor{8.,171.,255.},
	//		ofColor{82.f,195.f,253.f},
	//		ofColor{8.,171.,255.},
	//	}};

		// red, orange, yellow
		std::array<ofFloatColor,4> preBigbangColors{{
			ofColor{237.f,33.f,33.f},
			ofColor{255.,120.,0.},
			ofColor{255.f,236.f,27.f},
			ofColor{237.f,33.f,33.f},
		}};
		std::array<ofFloatColor,4> postBigBangColors;

		ofParameter<bool> runSimulation{ "Run Simulation", true };
		ofParameter<float> cameraDistance{"camera disntace", 0, 0, 10};
		ofParameter<void> triggerBigbang{"trigger bigbang"};
		ofParameter<bool> controlCamera{ "Control Camera", false };
		ofParameter<float> rotationRadius{"Rotation radius", 1, 0.5, 100, ofParameterScale::Logarithmic};
		ofParameter<float> rotationSpeed{"Rotation speed", 1, 0.1, 100, ofParameterScale::Logarithmic};
		ofParameter<float> state{"state", 0, 0, 5};

		struct : ofParameterGroup{
			ofParameter<ofFloatColor> color1{"color 1", ofColor::white};
			ofParameter<ofFloatColor> color2{"color 2", ofColor::white};
			ofParameter<ofFloatColor> color3{"color 3", ofColor::white};
			ofParameter<ofFloatColor> color4{"color 4", ofColor::white};
			PARAM_DECLARE("Colors",
			  color1,
			  color2,
			  color3,
			  color4)
		} colors;

		struct : ofParameterGroup{
			ofParameter<float> newClusterAt{ "New cluster at scale", 3, 1, 30};
			ofParameter<float> bigBangDuration{ "BigBang duration", 0.25f, 0.0f, 2.f};
			ofParameter<float> preBigBangWobbleDuration{ "Pre BigBang wobble duration", 3.f, 0.0f, 5.f};
			ofParameter<float> bbFlashStart{"bigbang flash start %", 0.9, 0.f, 1.f};
			ofParameter<float> bbFlashIn{"bigbang flash in, duration sec.", 0.1, 0.f, 1.f};
			ofParameter<float> bbFlashPlateau{"bigbang flash plateau, sec.", 0.1, 0.f, 1.f};
			ofParameter<float> bbFlashOut{"bigbang flash out, duration sec.", 0.1, 0.f, 1.f};
			ofParameter<float> bbTransitionFlash{"inflation transition flash at scale.", 10.f, 2.f, 20.f};
			ofParameter<float> bbTransitionIn{"inflation transition in, duration sec.", 0.5, 0.f, 1.f};
			ofParameter<float> bbTransitionPlateau{"inflation transition plateau, sec.", 0.3, 0.f, 1.f};
			ofParameter<float> bbTransitionOut{"inflation transition out, sec.", 2, 0.f, 3.f};
			ofParameter<ofFloatColor> bbTransitionColor{"inflation transition bg color", ofFloatColor::fromHex(0x91a5a3,1)};
			ofParameter<float> transitionParticlesDuration{"transition particles in. (s)", 1, 0, 5};
			ofParameter<float> transitionBlobsOutDuration{"transition blobs out. (s)", 1, 0, 5};
			ofParameter<float> HtBB{ "Rate of expansion at bigbang", 5.f, 1.f, 100.f}; // rate of expansion
			ofParameter<float> HtPostBB{ "Rate of expansion after bigbang", 0.05f, 0.0f, 5.f}; // rate of expansion
			ofParameter<float> Ht{ "Current rate of expansion", 5.f, 0.0f, 100.f}; // rate of expansion
			ofParameter<float> hubbleWavelength{ "Hubble (min) wavelength for any octave", 4.f, 0.01f, 4.f };
			ofParameter<int> geometrySize{"geometry size", 0, 0, 1024*1024*1024};

			PARAM_DECLARE("Equations",
				  bigBangDuration,
				  newClusterAt,
				  preBigBangWobbleDuration,
				  bbFlashStart,
				  bbFlashIn,
				  bbFlashPlateau,
				  bbFlashOut,
				  bbTransitionFlash,
				  bbTransitionIn,
				  bbTransitionPlateau,
				  bbTransitionOut,
				  bbTransitionColor,
				  transitionParticlesDuration,
				  transitionBlobsOutDuration,
				  HtBB,
				  HtPostBB,
				  Ht,
				  hubbleWavelength,
				  geometrySize);
		} equations;

		struct : ofParameterGroup
		{
			ofParameter<bool> debug{ "Debug Noise", false };
			ofParameter<bool> renderBack{ "Render Back", true };
			ofParameter<bool> renderFront{ "Render Front", false };
			ofParameter<bool> boxBackRender{ "Render Box Back", false };
			ofParameter<int>  fps{ "fps", 60, 20, 1200, ofParameterScale::Logarithmic};
			ofParameter<bool> record{"Record image seq.",false};
			ofParameter<bool> recordVideo{"Record video",false};

			PARAM_DECLARE("Render",
				debug,
				renderBack,
				renderFront,
				boxBackRender,
				fps,
				record,
				recordVideo);
		} render;

		PARAM_DECLARE("Inflation",
			runSimulation,
			cameraDistance,
			controlCamera,
			rotationRadius,
			rotationSpeed,
			state,
			render,
			colors,
			equations);
	} parameters;

	ofEasyCam camera;
	void resetWavelengths();
	void resetWavelength(size_t octave);

	ofFbo fbo, postFbo;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParameters;

	ofxPanel gui;
	ofxTextureRecorder recorder;
	bool rotating = false;
	ofPolyline cameraPath;
	glm::vec3 dofTarget;
	double dofTimeStart = 0;
	float dofDistanceTarget;
	float dofDistanceStart;

	ofParameter<string> textParam{"textParam"};
	ofxInputField<string> textField;

	std::vector<ofEventListener> listeners;
	ofxTimeline timeline;
	bool showTimeline = true;
	int prevState = 0;
	float orbitAngle;
};
