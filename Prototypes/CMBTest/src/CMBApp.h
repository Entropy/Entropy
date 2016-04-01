#pragma once

//#define COMPUTE_OPENCL 1
#define COMPUTE_GLSL 1

//#define THREE_D 1

#define USE_CUSTOM_FBO 1

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxVolumetrics.h"
#ifdef COMPUTE_OPENCL
#include "MSAOpenCL.h"
#ifdef THREE_D
#include "OpenCLImage3D.h"
#endif
#endif
#ifdef COMPUTE_GLSL
#ifdef USE_CUSTOM_FBO
#include "ofxFbo.h"
#endif
#endif

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

#ifdef COMPUTE_OPENCL
        msa::OpenCL openCL;
        msa::OpenCLKernelPtr dropKernel;
        msa::OpenCLKernelPtr ripplesKernel;
        msa::OpenCLKernelPtr copyKernel;
#ifdef THREE_D
        OpenCLImage3D clImages[2];
        OpenCLImage3D clImageTmp;
#else
        msa::OpenCLImage clImages[2];
        msa::OpenCLImage clImageTmp;
#endif
#endif

#ifdef COMPUTE_GLSL
        ofShader shader;
        ofVboMesh mesh;
        ofCamera orthoCamera;

#ifdef USE_CUSTOM_FBO
        ofxFbo fbos[2];
		ofxFbo tmp;

#ifdef THREE_D
        ofxTexture3d textures[2];
#else
        ofTexture textures[2];
		ofTexture textureTmp;
#endif  // THREE_D
#else
        ofFbo fbos[2];
        ofFbo tmp;
#endif  // USE_CUSTOM_FBO
#endif  // COMPUTE_GLSL

#ifdef THREE_D
        ofVec3f dimensions;

        ofEasyCam cam;
        ofxVolumetrics volumetrics;
#else
        ofVec2f dimensions;
#endif

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

        // GUI
        void imGui();

        ofxImGui gui;
        bool bGuiVisible;
        bool bMouseOverGui;
    };
}
