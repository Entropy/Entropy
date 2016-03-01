#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

struct BlackHole
{
    ofVec3f position;
    float radius;
    float ringRadiusInner;
    float ringRadiusOuter;
    float ringThickness;
    float mass;
};

class ofApp : public ofBaseApp
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

    BlackHole gargantua;
    int samplesPerPixel;

    ofImage noiseTex;
    ofImage spaceTex;
    
    ofShader renderShader;
    ofPlanePrimitive plane;
    ofVboMesh mesh;

    ofEasyCam camera;
    float cameraFOV;
    float cameraSensitivity;

    // GUI
    void imGui();

    ofxImGui gui;
    bool bGuiVisible;
    bool bMouseOverGui;
};
