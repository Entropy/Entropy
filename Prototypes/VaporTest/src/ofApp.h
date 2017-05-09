#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTimeline.h"

#include "SequenceRamses.h"
#include "ofxTextureRecorder.h"

class ofApp 
	: public ofBaseApp
{
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

    ent::SequenceRamses m_sequenceRamses;

    ofEasyCam m_camera;

	ofParameter<float> m_scale{"world scale", 1, 0.1, 100};
	ofParameter<bool> m_glDebug{"debug gl", false};
	ofParameter<bool> m_showOctree{"show octree", false};
	ofParameter<bool> m_showOctreeDensities{"show octree densities", false};
	ofParameter<bool> m_showAxis{"show axis", false};
	ofParameter<bool> m_vboTex{"vbo texture", false};
	ofParameter<bool> m_bSyncPlayback{"sync playback", false};
	ofParameter<bool> m_bExportFrames{"record", false};
	ofParameter<string> m_exportPath{ofToDataPath("",true)};
	ofParameterGroup appParameters{
		"application",
		m_scale,
		m_glDebug,
		m_showOctree,
		m_showOctreeDensities,
		m_showAxis,
		//m_vboTex,
		m_bSyncPlayback,
		m_bExportFrames,
		m_exportPath,
	};


	ofParameterGroup parameters{
		"vapor",
		appParameters,
		m_sequenceRamses.parameters,
	};

	ofxTimeline m_timeline;
	ofxTLCameraTrack *m_cameraTrack;

	ofxPanel m_gui;
	bool m_bGuiVisible = true;
	ofxTextureRecorder recorder;

	ofTrueTypeFont ttf;
};
