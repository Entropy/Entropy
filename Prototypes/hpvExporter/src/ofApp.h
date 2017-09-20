#pragma once

#include "ofBaseApp.h"
#include "ofxHPVPlayer.h"
#include "ofxVideoRecorder.h"
#include "ofxTextureRecorder.h"

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


	ofxHPVPlayer player;
	std::unique_ptr<ofxTextureRecorder> recorder;
	std::deque<std::filesystem::path> filesToOpen;
	size_t currentFrame = 0;
	std::string currentVideo;
	bool done = true;
	ofFbo fbo;
};
