#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTimeline.h"

#include "entropy/geom/Sphere.h"
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

	ofParameter<float> nearClip{ "Near Clip", 0.03f, 0.01f, 5000.0f };
	ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.01f, 5000.0f };
	ofParameterGroup cameraParams{ "Camera",
		nearClip,
		farClip };

	ofParameter<bool> renderBoss{ "Render BOSS", true };
	ofParameter<bool> renderDes{ "Render DES", false };
	ofParameter<bool> renderVizir{ "Render ViziR", false };
	ofParameter<float> scale{ "Scale", 1.0f, 0.01f, 100.0f };
	ofParameter<float> pointSize{ "Point Size", 8.0f, 0.01f, 32.0f };
	ofParameterGroup renderParams{ "Render",
		renderBoss,
		renderDes,
		renderVizir,
		scale,
		pointSize };

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;

	entropy::surveys::DataSet dataSetBoss;
	entropy::surveys::DataSet dataSetDes;
	entropy::surveys::DataSet dataSetVizir;

	entropy::geom::Sphere sphereGeom;
	ofTexture sphereTexture;
	ofShader sphereShader;

	ofShader spriteShader;
	ofTexture spriteTexture;

	ofEasyCam camera;
};
