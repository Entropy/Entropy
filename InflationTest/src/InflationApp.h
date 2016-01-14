#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMarchingCubes.h"

#include "NoiseField.h"

namespace entropy
{
    class InflationApp : public ofBaseApp
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

        ofEasyCam camera;
        ofLight light;

        // Marching Cubes
        void paramsMarchingCubesChanged(ofAbstractParameter& param);

        ofxMarchingCubes marchingCubes;

        ofxPanel panelMarchingCubes;

        ofParameterGroup paramsMarchingCubes;
        ofParameter<int> mcResX;
        ofParameter<int> mcResY;
        ofParameter<int> mcResZ;
        ofParameter<float> mcScale;
        ofParameter<float> mcThreshold;
        ofParameter<bool> mcFlipNormals;
        ofParameter<bool> mcSmooth;

        // Noise Field
        NoiseField noiseField;

        ofxPanel panelNoiseField;

        // Render
        ofShader normalShader;
        
        ofxPanel panelRender;
        
        ofParameterGroup paramsRender;
        ofParameter<bool> debug;
        ofParameter<bool> drawGrid;
        ofParameter<bool> wireframe;
        ofParameter<bool> shadeNormals;

        // GUI
        bool guiVisible;
    };
}
