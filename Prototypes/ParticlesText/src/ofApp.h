#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "WireframeFillRenderer.h"
#include "PostEffects.h"
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


		std::vector<glm::vec3> positions;
		std::vector<int> types;
		std::vector<std::pair<size_t,size_t>> relations;
		ofEasyCam cam;

		entropy::render::WireframeFillRenderer renderer;
		entropy::render::PostEffects postEffects;
		entropy::render::PostParameters postParameters;

		ofParameter<float> maxDistance{"max distance", 1, 0, 4};
		ofParameter<float> relDistance{"relation distance", 0.05f, 0, 2};
		ofParameter<float> fulltextDistance{"fulltext distance", 0.05f, 0, 2};

		ofParameterGroup parameters{
			"parameters",
			maxDistance,
			relDistance,
			fulltextDistance,
			postParameters,
			renderer.parameters,
		};

		ofxPanel gui{parameters};

		std::vector<ofEventListener> listeners;
		float side = 768;

		ofFbo fbo, fbo2;
		ofTrueTypeFont font;

		ofxTextureRecorder recorder;
		bool save = false;
};
