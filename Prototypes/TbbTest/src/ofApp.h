#pragma once

#include "ofMain.h"
#include "Octree.h"
#include "ParticleSystem.h"
#include "ofxImGui.h"
#include "ofxPersistent.h"

class ofApp : public ofBaseApp
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

private:
    void drawGui();
    void guiFromPersistent(ofxPersistent& persistent);
    ofxImGui gui;
    ofxPersistent persistent;
    
    ofEasyCam cam;
    //ofVboMesh mesh;
    nm::ParticleSystem particleSystem;
    
    /*
    nm::Octree<nm::Particle> octree;
    nm::Particle* particles;
    unsigned numParticles;
     */
};
