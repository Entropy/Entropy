#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTextureRecorder.h"
#include "ofxTimeline.h"

#include "entropy/Helpers.h"
#include "entropy/geom/Sphere.h"
#include "entropy/render/PostEffects.h"
#include "entropy/render/WireframeFillRenderer.h"
#include "entropy/surveys/DataSet.h"
#include "entropy/surveys/Travel.h"

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

	void reset();

	glm::mat4 getWorldTransform() const;

	bool loadPreset(const string & presetName);
	bool savePreset(const string & presetName);

protected:
	static const string kSceneName;

	struct : ofParameterGroup
	{
		ofParameter<float> worldScale{ "World Scale", 1.0f, 0.01f, 100.0f, ofParameterScale::Logarithmic };
		ofParameter<float> orbitSpeed{ "Orbit Speed", 0.0f, -100.0f, 100.0f };

		struct : ofParameterGroup
		{
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 5000.0f };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.01f, 5000.0f };
			ofParameter<float> fov{ "FOV", 60, 1, 120 };

			PARAM_DECLARE("Camera",
				nearClip,
				farClip,
				fov);
		} camera;

		struct : ofParameterGroup
		{
			ofParameter<bool> recordSequence{ "Record Sequence", false };
			ofParameter<bool> recordVideo{ "Record Video", false };
			ofParameter<int> renderWidth{ "Render Width", 4200, 1920, 5760 };
			ofParameter<int> renderHeight{ "Render Height", 1080, 360, 1080 };

			PARAM_DECLARE("Recording",
				recordSequence,
				recordVideo,
				renderWidth,
				renderHeight);
		} recording;

		PARAM_DECLARE("Scene",
			worldScale,
			orbitSpeed,
			camera,
			recording);
	} parameters;

	entropy::surveys::DataSet::SharedParams sharedParams;

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;
	ofxTLCameraTrack cameraTrack;
	bool guiVisible;
	bool timelineVisible;

	ofxTextureRecorder textureRecorder;

	std::string currPreset;

	entropy::surveys::DataSet dataSetBoss;
	entropy::surveys::DataSet dataSetDes;
	entropy::surveys::DataSet dataSetVizir;

	entropy::surveys::Travel travel;

	entropy::geom::Sphere sphereGeom;
	ofTexture sphereTexture;
	ofShader sphereShader;
	ofShader::Settings sphereSettings;
	std::time_t sphereTime;

	ofShader spriteShader;
	std::time_t spriteTime;
	ofTexture spriteTexture;

	ofShader shellShader;
	ofShader::Settings shellSettings;
	std::time_t shellTime;
	ofTexture shellTexture;

	ofShader modelShader;
	ofShader::Settings modelSettings;
	std::time_t modelTime;
	ofBufferObject dataBuffer;
	ofVboMesh masterMesh;
	ofVboMesh scaledMesh;

	ofEasyCam camera;
	glm::vec3 orbitOffset;

	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	ofFbo fboScene;
	ofFbo fboPost;
};
