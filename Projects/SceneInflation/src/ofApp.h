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
	double dt = 0.0;
	double t_bigbang = 0.0;
	double t_from_bigbang = 0.0;
	double t_expansion = 0.0;
	double t_from_expansion = 0.0;
	double t_from_particles = 0.0;


	struct Cluster{
		float scale = 1;
		glm::vec3 origin;
		double startTime;
		float startScale = 1;
		bool negativeSpace = false;
		bool negativeWire = true;
		float negativeFill;
		glm::quat rotation;
		float alpha = 1;
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
		ofParameter<void> offsetTimeline{"offset timeline"};
		ofParameter<void> offsetSelected{"offset selected"};
		ofParameter<void> triggerBigbang{"trigger bigbang"};
		ofParameter<bool> controlCamera{ "Control Camera", false };
		ofParameter<bool> rotating{"Rotating", true};
		ofParameter<bool> rotatingUseSpeed{"Rotation use speed", true};
		ofParameter<float> rotationRadius{"Rotation radius", 1, 0.05, 5, ofParameterScale::Logarithmic};
		ofParameter<float> rotationRadiusSpeed{"Rotation radius speed", 0.1, -1, 1, ofParameterScale::Logarithmic};
		ofParameter<float> rotationSpeed{"Rotation speed", 1, 0.1, 100, ofParameterScale::Logarithmic};
		ofParameter<float> distanceToTrankingParticle{"Distance to tracking particle", 1, 0.1, 100, ofParameterScale::Logarithmic};
		ofParameter<float> state{"state", 0, 0, 5};
		ofParameter<bool> enableCooldown{"Enable cooldown renderer", false};
		ofParameter<bool> showCooldown{"Show cooldown renderer", false};
		ofParameter<bool> showReheating{"show reheating", false};
		ofParameter<bool> onlyReheating{"only reheating", false};

		struct : ofParameterGroup{
			ofParameter<ofFloatColor> color1{"color 1", ofColor::white};
			ofParameter<ofFloatColor> color2{"color 2", ofColor::white};
			ofParameter<ofFloatColor> color3{"color 3", ofColor::white};
			ofParameter<ofFloatColor> color4{"color 4", ofColor::white};
			ofParameter<float> alphaScaleStart{"alpha start scale", 0.3, 0, 2};
			ofParameter<float> alphaScaleEnd{"alpha end scale", 0.5, 0, 5};
			ofParameter<bool> negativeWire{"negative wire", false};
			PARAM_DECLARE("Colors",
			  color1,
			  color2,
			  color3,
			  color4,
			  alphaScaleStart,
			  alphaScaleEnd,
			  negativeWire)
		} colors;

		struct : ofParameterGroup{
			ofParameter<ofFloatColor> color1{"color 1", ofFloatColor::black};
			ofParameter<ofFloatColor> color2{"color 2", ofFloatColor::black};
			ofParameter<ofFloatColor> color3{"color 3", ofFloatColor::darkRed};
			ofParameter<ofFloatColor> color4{"color 4", ofFloatColor::brown};
			ofParameter<bool> negativeWire{"negative wire", false};
			PARAM_DECLARE("Negative colors",
			  color1,
			  color2,
			  color3,
			  color4,
			  negativeWire)
		} negativeColors;

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
		} coolDownColors;

		struct : ofParameterGroup{
			ofParameter<ofFloatColor> color1{"color 1", ofColor::white};
			ofParameter<ofFloatColor> color2{"color 2", ofColor::white};
			ofParameter<ofFloatColor> color3{"color 3", ofColor::white};
			ofParameter<ofFloatColor> color4{"color 4", ofColor::white};
			PARAM_DECLARE("Reheating",
			  color1,
			  color2,
			  color3,
			  color4)
		} reheatingColors;

		struct : ofParameterGroup{
			ofParameter<void> newCluster{"Trigger new cluster"};
			ofParameter<void> newNegativeCluster{"Trigger new negative cluster"};
			ofParameter<float> newClusterAt{ "New cluster at scale", 3, 1, 30};
			ofParameter<float> mainClusterScaleAfterBigbang{ "Main cluster scale after bb", 0, 0, 30, ofParameterScale::Logarithmic};
			ofParameter<float> transitionParticlesDuration{"transition particles in. (s)", 1, 0, 5};
			ofParameter<float> transitionBlobsOutDuration{"transition blobs out. (s)", 1, 0, 5};
			ofParameter<float> Ht{ "Current rate of expansion", 5.f, 0.0f, 100.f}; // rate of expansion
			ofParameter<float> hubbleWavelength{ "Hubble (min) wavelength for any octave", 4.f, 0.01f, 4.f };
			ofParameter<float> maxClusterScale{"Max scale for any cluster", 30, 0, 60};

			PARAM_DECLARE("Equations",
				  newCluster,
				  newNegativeCluster,
				  newClusterAt,
				  mainClusterScaleAfterBigbang,
				  transitionParticlesDuration,
				  transitionBlobsOutDuration,
				  Ht,
				  hubbleWavelength,
				  maxClusterScale);
		} equations;

		struct : ofParameterGroup
		{
			ofParameter<bool> debug{ "Debug Noise", false };
			ofParameter<int>  fps{ "fps", 60, 20, 1200, ofParameterScale::Logarithmic};
			ofParameter<bool> systemClock{"system clock", false};
			ofParameter<bool> record{"Record image seq.",false};
			ofParameter<bool> recordVideo{"Record video",false};

			PARAM_DECLARE("Render",
				debug,
				fps,
				systemClock,
				record,
				recordVideo);
		} render;

		struct : ofParameterGroup{
			ofParameter<float> pct{ "circle pct", 0, 0, 1};
			ofParameter<float> inner{ "inner radius", 0.8, 0, 1};
			ofParameter<float> outer{ "outer radius", 0.9, 0, 1};

			PARAM_DECLARE("Circle",
						  pct,
						  inner,
						  outer);
		} circle;

		PARAM_DECLARE("Inflation",
			runSimulation,
			offsetTimeline,
			offsetSelected,
			controlCamera,
			rotating,
			rotatingUseSpeed,
			rotationRadius,
			rotationSpeed,
			rotationRadiusSpeed,
			state,
			render,
			circle,
			colors,
			negativeColors,
			equations);
	} parameters;


	ofEasyCam camera;
	void resetWavelengths();
	void resetWavelength(size_t octave);

	ofFbo fbo, postFbo;
	ofFbo fboCooldown, postFboCooldown;
	ofFbo fboReheating, postFboReheating;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParameters;
	entropy::render::WireframeFillRenderer::Parameters coolDownParameters;
	entropy::render::WireframeFillRenderer::Parameters reheatingParameters;

	ofxPanel gui;
	ofxTextureRecorder recorder, recorderCooldown, recorderReheating;
	std::string recorderPath, videoRecorderPath;

	ofPolyline cameraPath;
	glm::vec3 dofTarget;
	double dofTimeStart = 0;
	float dofDistanceTarget;
	float dofDistanceStart;

	std::vector<ofEventListener> listeners;
	ofxTimeline timeline;
	bool showTimeline = true;
	int prevState = 0;
	float orbitAngle;

	bool forceRedraw = true;

	ofMesh circle;
	glm::vec3 particleToTrack;
};
