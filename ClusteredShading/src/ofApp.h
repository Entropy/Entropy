#pragma once

#include "ofMain.h"

#include "lb/math/Plane.h"
#include "lb/math/ProjectionMath.h"
#include "lb/math/PointLight.h"
#include "lb/gl/GLError.h"

#include "ClusterGrid.h"
#include "ClusterGridDebug.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();

    void SetupLights();
    void UpdatePointLightUBO();

    void AnimateLights();

    void DrawAllPointLights();
    void DrawCulledPointLights();
    void DrawClusteredPointLights();
    void DrawOccupiedClusters();

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
    std::vector<lb::PointLight> m_pointLights;
    lb::ClusterGrid             m_clusterGrid;
    lb::ClusterGridDebug        m_clusterGridDebug;
    ofBufferObject              m_pointLightUbo;

    ofShader                    m_shader;

    ofCamera                   m_camera;
    ofEasyCam                   m_debugCamera;

    ofSpherePrimitive           m_sphere;
};
