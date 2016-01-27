#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "NBodySystemCPU.h"
#include "NBodySystemOpenCL.h"
#include "ParticleRenderer.h"
#include "Preset.h"

#define USE_OPENCL 1

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

        NBodySystem *system;
        int numBodies;

        float* hPos;
        float* hVel;
        float* hColor;

        // Rendering

        ofEasyCam camera;

        ParticleRenderer *renderer;
        ParticleRenderer::DisplayMode displayMode;

        ofParameter<float> pointSize;
    };
}
