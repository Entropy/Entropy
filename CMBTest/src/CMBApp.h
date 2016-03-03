#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "MSAOpenCL.h"

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
        ofVboMesh mesh;
        ofFbo fbos[2];

        msa::OpenCL openCL;
        msa::OpenCLKernelPtr ripplesKernel;
        msa::OpenCLKernelPtr copyKernel;
        msa::OpenCLImage clImages[2];
        msa::OpenCLImage clImageTmp;

        ofFloatColor tintColor;
        ofFloatColor dropColor;

        bool bDropOnPress;
        bool bDropUnderMouse;
        int dropRate;

        float damping;
        float radius;

        bool bRestart;

        int activeIndex;

        // GUI
        void imGui();

        ofxImGui gui;
        bool bGuiVisible;
        bool bMouseOverGui;
    };
}
