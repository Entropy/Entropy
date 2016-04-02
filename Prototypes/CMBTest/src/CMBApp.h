#pragma once

//#define COMPUTE_OPENCL 1
#define COMPUTE_GLSL 1

#define THREE_D 1

#include "ofMain.h"
#include "ofxImGui.h"

#ifdef COMPUTE_OPENCL
#include "MSAOpenCL.h"
#ifdef THREE_D
#include "ofxVolumetrics3D.h"
#include "OpenCLImage3D.h"
#endif
#endif

#ifdef COMPUTE_GLSL
#include "ofxFbo.h"
#ifdef THREE_D
#include "ofxVolumetricsArray.h"
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
        OpenCLImage3D clImages[3];
#else
        msa::OpenCLImage clImages[3];
#endif
#endif

#ifdef COMPUTE_GLSL
#ifdef THREE_D
		ofShader dropShader;
		ofShader copyShader;
#endif
        ofShader ripplesShader;
        ofVboMesh mesh;

        ofxFbo fbos[3];

#ifdef THREE_D
        ofxTextureArray textures[3];
#else
        ofTexture textures[3];
#endif  // THREE_D
#endif  // COMPUTE_GLSL

#ifdef THREE_D
        ofVec3f dimensions;

        ofEasyCam cam;
#ifdef COMPUTE_OPENCL
        ofxVolumetrics3D volumetrics;
#endif
#ifdef COMPUTE_GLSL
		ofxVolumetricsArray volumetrics;
#endif
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
