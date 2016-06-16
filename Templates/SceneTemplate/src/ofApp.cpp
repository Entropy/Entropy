#include "ofApp.h"

#include "entropy/scene/Manager.h"
#include "entropy/scene/Template.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	auto sceneTemplate = make_shared<entropy::scene::Template>();
	entropy::scene::Manager::X()->addScene(sceneTemplate);
	entropy::scene::Manager::X()->setCurrentScene(sceneTemplate->getName());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	entropy::scene::Manager::Destroy();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

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
