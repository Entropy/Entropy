#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxVolumetrics.h"
#include "MSAOpenCL.h"
#include "OpenCLImage3D.h"

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

//        ofShader shader;
//        ofVboMesh mesh;
//        ofFbo fbos[2];

        ofVec3f dimensions;

        msa::OpenCL openCL;
        msa::OpenCLKernelPtr dropKernel;
        msa::OpenCLKernelPtr ripplesKernel;
        msa::OpenCLKernelPtr copyKernel;
        OpenCLImage3D clImages[2];
        OpenCLImage3D clImageTmp;

        ofFloatColor tintColor;
        ofFloatColor dropColor;

        bool bDropOnPress;
        bool bDropUnderMouse;
        int dropRate;

        float damping;
        float radius;
        float ringSize;

        bool bRestart;

        int activeIndex;

        ofEasyCam cam;
        ofxVolumetrics volumetrics;

        // GUI
        void imGui();

        ofxImGui gui;
        bool bGuiVisible;
        bool bMouseOverGui;
    };
}
