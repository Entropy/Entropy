#pragma once

#include "ofMain.h"

#include "PerViewUbo.h"
#include "lb/lighting/LightSystem.h"
#include "lb/lighting/PointLight.h"
#include "lb/camera/CameraTools.h"
#include "lb/gl/DDSCubeMap.h"

class ofApp : public ofBaseApp {

enum class AppMode
{
    NORMAL_VIEW = 0,
    DEBUG_VIEW  = 1
};

public:
    void setup();
    void update();
    void draw();

    void SetupLighting();

    void CreateRandomLights();
    void AnimateLights();

    void SetAppMode( const AppMode _mode );

    void DrawScene();


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

private:
    AppMode                     m_appMode;

    lb::ViewUbo                 m_viewUbo;
    lb::LightSystem             m_lightSystem;
    lb::CameraParams            m_cameraParams;

    lb::DDSCubeMap              m_cubeMap;

    ofShader                    m_shader;

    ofEasyCam                   m_camera;
    ofEasyCam                   m_debugCamera;

    ofSpherePrimitive           m_sphere;
};
