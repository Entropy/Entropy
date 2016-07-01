#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    //ofSetFrameRate(480);
    ofSetVerticalSync(false);
    
    ofBackground(0);
    
    octree.init(ofVec3f(-400.f), ofVec3f(400.f));
    for (unsigned i = 0; i < 100000; ++i)
    {
        /*mesh.addVertex(ofVec3f(ofRandom(-200.f, 200.f),
                               ofRandom(-200.f, 200.f),
                               ofRandom(-200.f, 200.f)));*/
        mesh.addVertex(ofVec3f(400.f * ofSignedNoise(i / 2000.f, 10),
                               400.f * ofSignedNoise(i / 2000.f, 1e-6),
                               400.f * ofSignedNoise(i / 2000.f, 1e6)));
        octree.addPoint(mesh.getVertices().back());
    }
    
    wireFrame = ofMesh::box(10.f, 10.f, 10.f, 1.f, 1.f, 1.f);
}

//--------------------------------------------------------------
void ofApp::update()
{
    octree.clear();
    octree.addPointsParallel(mesh.getVertices());
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    cam.begin();
    ofPushStyle();
    ofSetColor(255);
    mesh.drawVertices();
    ofSetColor(255, 0, 0);
    octree.debugDraw();
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
