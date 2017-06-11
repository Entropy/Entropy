#pragma once

#include "ofMain.h"
#include "ofxGui.h"

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

		std::vector<ofTexture> textures;
		ofParameter<float> textureSeparation{"texture separation", 0.05, 0, 1};
		ofParameter<float> frequency0{"frequency0", 2, 0, 1000};
		ofParameter<float> frequency1{"frequency1", 4, 0, 1000};
		ofParameter<float> frequency2{"frequency2", 8, 0, 1000};
		ofParameter<float> frequency3{"frequency3", 16, 0, 1000};
		ofParameter<float> colorramp_low{"color ramp low", 0.35, 0, 1};
		ofParameter<float> colorramp_high{"color ramp high", 1, 0, 1};
		ofParameterGroup parameters{
			"params",
			textureSeparation,
			        frequency0,
			        frequency1,
			        frequency2,
			        frequency3,
			        colorramp_low,
			        colorramp_high,
		};

		ofxPanel gui{parameters};
		ofCamera camera;
};
