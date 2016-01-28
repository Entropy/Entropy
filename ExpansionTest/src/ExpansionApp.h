#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "ofxHeadCamera.h"

namespace entropy
{
    class ExpansionParticle
    {
    public:
        enum Type
        {
            TypeCamera = 0,
            TypeParticle
        };

        void setup(float size);
        void update(float scale);

        int32_t type;
        float size;

        ofVec3f ini;
        ofVec3f pos;
    };

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

        void restart();

        ofxPanel guiPanel;
        bool bGuiVisible;

        ofParameterGroup paramGroup;

        ofParameter<bool> bRestart;

        ofEasyCam easyCam;
        ofxHeadCamera camera;
        ofParameter<bool> bDebugCamera;

        ofBoxPrimitive box;
        float scale;
        ofParameter<float> size;
        ofParameter<int> resolution;
        ofParameter<float> speed;
        ofParameter<bool> bDrawGrid;

        vector<ExpansionParticle> particles;
        ofParameter<float> numParticles;

        ofVboMesh vboMesh;
        ofShader shader;
        ofTexture texture;

//        ofParameter<ofColor> tintColor;
//        ofParameter<ofColor> dropColor;
//
//        ofParameter<bool> bDropOnPress;
//        ofParameter<bool> bDropUnderMouse;
//        ofParameter<int> dropRate;
//
//        ofParameter<float> damping;
//        ofParameter<float> radius;

    };
}
