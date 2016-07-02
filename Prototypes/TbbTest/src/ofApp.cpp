#include "ofApp.h"
#include "ofxObjLoader.h"

//#define _TEAPOT

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetFrameRate(60);
    //ofSetVerticalSync(false);
    
    ofBackground(0);
    nm::Octree<ofVec3f>::setMaxDepth(5);
    //octree.init(ofVec3f(-400.f), ofVec3f(400.f));
    particleSystem.init(ofVec3f(-400.f), ofVec3f(400.f));
    
#ifdef _TEAPOT
    ofxObjLoader::load("teapot.obj", mesh);
    for (auto& v : mesh.getVertices())
    {
        v *= 80.f;
        v.y -= 80.f;
        octree.addPoint(v);
    }
#else
    for (unsigned i = 0; i < nm::ParticleSystem::MAX_PARTICLES; ++i)
    {
        mesh.addVertex(ofVec3f(400.f * ofSignedNoise(i / 2000.f, 10),
                               400.f * ofSignedNoise(i / 2000.f, 1e-6),
                               400.f * ofSignedNoise(i / 2000.f, 1e6)));
        particleSystem.addParticle(mesh.getVertices().back());
    }
    
    /*
    numParticles = 100000;
    particles = new nm::Particle[numParticles]();
    for (unsigned i = 0; i < numParticles; ++i)
    {
        //mesh.addVertex(ofVec3f(ofRandom(-200.f, 200.f),
        //                       ofRandom(-200.f, 200.f),
        //                       ofRandom(-200.f, 200.f)));
        mesh.addVertex(ofVec3f(400.f * ofSignedNoise(i / 2000.f, 10),
                               400.f * ofSignedNoise(i / 2000.f, 1e-6),
                               400.f * ofSignedNoise(i / 2000.f, 1e6)));
        particles[i].set(mesh.getVertices().back());
        octree.addPoint(particles[i]);
    }*/
#endif
}

//--------------------------------------------------------------
void ofApp::update()
{
    particleSystem.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    cam.begin();
    ofPushStyle();
    ofSetColor(255);
    //mesh.drawVertices();
    //mesh.draw();
    ofSetColor(255, 0, 0);
    //octree.debugDraw();
    ofPopStyle();
    cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
