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
        ofMaterial material;

        // Marching Cubes
        void paramsMarchingCubesChanged(ofAbstractParameter& param);

        ofxMarchingCubes marchingCubes;

        ofParameterGroup paramsMarchingCubes;
        ofParameter<int> resolution;
        ofParameter<float> scale;
        ofParameter<float> threshold;
        ofParameter<bool> radialClip;
        ofParameter<bool> fillEdges;
        ofParameter<bool> flipNormals;
        ofParameter<bool> smooth;

		ofxPanel panelMarchingCubes;

        // Noise Field
		NoiseField noiseField;

		ofxPanel panelNoiseField;

        // Render
        ofShader normalShader;

        ofParameterGroup paramsRender;
        ofParameter<bool> debug;
        ofParameter<bool> drawGrid;
        ofParameter<bool> wireframe;
        ofParameter<bool> shadeNormals;

		ofxPanel panelRender;

        // GUI
		bool guiVisible = true;
		ofParameter<bool> simulationRunning;
    };
}
