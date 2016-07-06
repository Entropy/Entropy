#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxTimeline.h"

#include "SequenceRamses.h"

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

    float m_scale;
	bool m_bSyncPlayback;

	bool m_bExportFrames;
	std::string m_exportPath;

    // GUI
    bool imGui();

	ofxTimeline m_timeline;
	ofxTLCameraTrack *m_cameraTrack;

    ofxImGui m_gui;
    bool m_bGuiVisible;
    bool m_bMouseOverGui;
	bool m_glDebug;
	bool m_showOctree;
	bool m_doFXAA;

	size_t z;
	ofFbo fullQuadFbo;
	ofShader fxaaShader;
	ofVboMesh fullQuad;
};
