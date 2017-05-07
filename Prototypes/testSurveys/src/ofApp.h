#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "entropy/render/PostEffects.h"
#include "entropy/render/WireframeFillRenderer.h"

class ofApp : public ofBaseApp{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofVboMesh galaxy;
	ofEasyCam camera;
	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParameters;
	ofFbo fbo1, fbo2;

	ofParameterGroup parameters{
		"surveys",
		renderer.parameters,
		postParameters,
	};
	ofxPanel gui{parameters};

};
