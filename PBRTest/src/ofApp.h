#pragma once

#include "ofMain.h"

#include "PerViewUbo.h"
#include "lb/lighting/LightSystem.h"
#include "lb/lighting/PointLight.h"
#include "lb/gl/CubeMapTexture.h"
#include "PBRMaterial.h"

#include "ofxImGui.h"

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

    void ClearPointLights();
    void ClearDirectionalLights();

    void AnimateLights();

    void SetAppMode( const AppMode _mode );

    void DrawSkybox();
    void DrawScene();
    void DrawSphereGrid();

    void imGui();



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

    lb::CubeMapTexture          m_radianceMap;
    lb::CubeMapTexture          m_irradianceMap;
    lb::CubeMapTexture          m_skyboxMap;

    PBRMaterial                 m_material;

    ofShader                    m_shader;
    ofShader                    m_skyboxShader;
    GLuint                      m_defaultVao;

    ofShader                    m_integrateDFGShader;
    ofFbo                       m_dfgFbo;

    float                       m_exposure;
    float                       m_gamma;

    ofEasyCam                   m_camera;
    ofEasyCam                   m_debugCamera;

    ofSpherePrimitive           m_sphere;

    ofxImGui                    m_gui;
    bool                        m_bMouseOverGui;
};
