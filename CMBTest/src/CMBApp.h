#pragma once

#include "ofMain.h"
#include "ofxGui.h"

namespace entropy
{
    class CMBApp : public ofBaseApp
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

        void restart();

        ofShader shader;
        ofFbo srcFbo, dstFbo;
        ofVboMesh mesh;

        float minDropHue, maxDropHue;
        float minDropSat, maxDropSat;
        float minDropBri, maxDropBri;

        ofColor tintColor;
        ofColor dropColor;

        bool bDropOnPress;
        int dropRate;

        float damping;
        float radius;
        
        bool bRestart;
    };
}
