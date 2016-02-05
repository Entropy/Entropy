#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxImGui.h"

#include "ofxHeadCamera.h"

namespace entropy
{
    class ExpansionApp : public ofBaseApp
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

        void imGui();

        ofxImGui gui;
        bool bGuiVisible;


        ofBoxPrimitive box;
        bool bRebuildBox;
        bool bDrawGrid;
        float size;
        int resolution;

        bool bReset;
        bool bPaused;
        int type;
        float speed;

        float dt;
        float scale;

        ofVboMesh vboMesh;
        ofShader shader;
        ofTexture texture;

        ofEasyCam easyCam;
        ofxHeadCamera headCam;
        int camera;

        float pointSize;
        bool bUseSprites;
    };
}
