#pragma once

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

	ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 5000.0f };
	ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.01f, 5000.0f };
	ofParameter<float> worldScale{ "World Scale", 1.0f, 0.01f, 100.0f, ofParameterScale::Logarithmic };
	ofParameterGroup parameters{ "Parameters",
		nearClip,
		farClip,
		worldScale };

	entropy::surveys::DataSet::SharedParams sharedParams;

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;

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

	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	ofFbo fboScene, fboPost;
};
