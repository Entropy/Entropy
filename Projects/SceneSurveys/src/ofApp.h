#pragma once

#include <unordered_set>

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTimeline.h"

#include "entropy/geom/Sphere.h"
#include "entropy/render/PostEffects.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/surveys/DataSet.h"

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

protected:
	static const string kSceneName;

	struct : ofParameterGroup
	{
		ofParameter<float> worldScale{ "World Scale", 1.0f, 0.01f, 100.0f, ofParameterScale::Logarithmic };

		struct : ofParameterGroup
		{
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 5000.0f };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.01f, 5000.0f };

			PARAM_DECLARE("Camera",
				nearClip,
				farClip);
		} camera;

		struct : ofParameterGroup
		{
			ofParameter<bool> enabled{ "Enable Orbit", false };
			ofParameter<float> speed{ "Speed", 0.0f, -100.0f, 100.0f };

			PARAM_DECLARE("Orbit",
				enabled,
				speed);
		} orbit;

		struct : ofParameterGroup
		{
			ofParameter<bool> enabled{ "Enable Travel", false };
			ofParameter<float> camCutoff{ "Cam Cutoff", 1.0f, 0.0f, 1000.0f };
			ofParameter<float> lookAtLerp{ "Look At Lerp", 0.2f, 0.0f, 1.0f };
			ofParameter<float> moveLerp{ "Move Lerp", 0.1f, 0.0f, 1.0f };

			PARAM_DECLARE("Travel",
				enabled,
				camCutoff,
				lookAtLerp,
				moveLerp);
		} travel;

		PARAM_DECLARE("Parameters",
			worldScale,
			camera,
			orbit,
			travel);
	} params;

	entropy::surveys::DataSet::SharedParams sharedParams;

	vector<ofEventListener> paramListeners;
	ofxPanel gui;
	ofxTimeline timeline;
	ofxTLCameraTrack cameraTrack;

	entropy::surveys::DataSet dataSetBoss;
	entropy::surveys::DataSet dataSetDes;
	entropy::surveys::DataSet dataSetVizir;

	entropy::geom::Sphere sphereGeom;
	ofTexture sphereTexture;
	ofShader sphereShader;
	ofShader::Settings sphereSettings;
	std::time_t sphereTime;

	ofShader spriteShader;
	std::time_t spriteTime;
	ofTexture spriteTexture;

	ofShader modelShader;
	ofShader::Settings modelSettings;
	std::time_t modelTime;
	ofBufferObject dataBuffer;
	ofVboMesh masterMesh;
	ofVboMesh scaledMesh;

	ofEasyCam camera;
	int prevTargetIndex;
	std::unordered_set<int> travelLog;

	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	ofFbo fboScene, fboPost;
};
