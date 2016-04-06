#pragma once

#include "ofMain.h"
#include "ofxImGui.h"
#include "ofxTimeline.h"

//#define COMPUTE_GL_2D 1
#define COMPUTE_GL_3D 1
//#define COMPUTE_CL_2D 1
//#define COMPUTE_CL_3D 1

#ifdef COMPUTE_GL_2D
#include "CmbSceneGL2D.h"
#elif defined(COMPUTE_GL_3D)
#include "CmbSceneGL3D.h"
#elif defined(COMPUTE_CL_2D)
#include "CmbSceneCL2D.h"
#elif defined(COMPUTE_CL_3D)
#include "CmbSceneCL3D.h"
#endif

namespace ent
{
    class ofApp 
		: public ofBaseApp
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

#ifdef COMPUTE_GL_2D
		CmbSceneGL2D m_cmbScene;
#elif defined(COMPUTE_GL_3D)
		CmbSceneGL3D m_cmbScene;
#elif defined(COMPUTE_CL_2D)
		CmbSceneCL2D m_cmbScene;
#elif defined(COMPUTE_CL_3D)
		CmbSceneCL3D m_cmbScene;
#endif

		glm::vec3 m_dimensionEditor;
		glm::vec3 m_dimensionExport;

		ofFloatColor m_backgroundColor; 
		ofFloatColor m_tintColor;

		ofEasyCam m_camera;

		// Export
		void beginExport();
		void endExport();

		bool m_bExportFrames;
		std::string m_exportPath;

        // GUI
        bool imGui();

		ofxTimeline m_timeline;
		ofxTLCameraTrack *m_cameraTrack;

        ofxImGui m_gui;
        bool m_bGuiVisible;
        bool m_bMouseOverGui;
    };
}
