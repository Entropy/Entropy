#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "NBodySystemCPU.h"
#include "NBodySystemOpenCL.h"
#include "ParticleRenderer.h"
#include "Preset.h"

namespace entropy
{
    class PartyCLApp : public ofBaseApp
    {
    public:
        void setup();
        void update();
        void draw();

        void keyPressed(int key);
//        void keyReleased(int key);
//        void mouseMoved(int x, int y );
//        void mouseDragged(int x, int y, int button);
//        void mousePressed(int x, int y, int button);
//        void mouseReleased(int x, int y, int button);
//        void mouseEntered(int x, int y);
//        void mouseExited(int x, int y);
//        void windowResized(int w, int h);
//        void dragEvent(ofDragInfo dragInfo);
//        void gotMessage(ofMessage msg);

        // App
        
        void paramsChanged(ofAbstractParameter& param);

        ofParameterGroup params;

        ofxPanel guiPanel;
        bool bGuiVisible;

        ofParameter<bool> bPaused;
        ofParameter<bool> bReset;

        // Simulation

        void randomizeBodies();
        void resetSimulation();
        void loadPreset();

        ofParameter<float> timestep;
        ofParameter<float> clusterScale;
        ofParameter<float> velocityScale;
        ofParameter<float> softening;
        ofParameter<float> damping;

        vector<Preset> presets;
        int presetIndex;
        NBodyConfig activeConfig;

        string filename;

        NBodySystem *system;
        int numBodies;

        vector<ofVec4f> hPos;
        vector<ofVec4f> hVel;
        vector<ofVec4f> hColor;

        // Rendering

        ofEasyCam camera;

        ParticleRenderer *renderer;
        ParticleRenderer::DisplayMode displayMode;

        ofParameter<float> pointSize;
    };
}
