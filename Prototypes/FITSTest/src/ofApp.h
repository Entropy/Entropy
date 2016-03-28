#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxImGui.h"

enum ExtraAttributes
{
    MASS_ATTRIBUTE = 5,
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

    void loadData(const string& filePath);

    void imGui();

    ofxImGui gui;
    bool bGuiVisible;

    float pointSize;
    bool bUseSprites;

    float scale;

    ofShader shader;
    ofVboMesh vboMesh;
    ofTexture texture;

    ofEasyCam cam;
};
