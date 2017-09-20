#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void reloadOctree();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void mouseScrolled(int x, int y, float scrollX, float scrollY );
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);


	ofImage image;
	ofEasyCam camera;
	ofMesh mesh;
	ofBuffer octreeSrc;
	float boxSize;
	std::filesystem::file_time_type lastModifed;
	ofFbo fbo;

	//ofParameter<float> scale{"scale", 1000, 0.01, 100000, ofParameterScale::Logarithmic};
	ofParameter<float> crop{"crop", 0, 0, 100000, ofParameterScale::Logarithmic};
	ofParameter<float> rotation{"rotation",0, 0, 360};
	ofParameter<float> fov{"fov",30, 0.1, 60};
	ofParameter<float> cameraMagnification{"cameraMagnification",1,0.1,100};
	ofParameterGroup parameters{
		"parameters",
		crop,
		//scale,
		rotation,
		fov,
		cameraMagnification,
	};
	ofxPanel gui{parameters};
};
