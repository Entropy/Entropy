#include "ofApp.h"

#include "entropy/Helpers.h"
#include "entropy/scene/PopUps.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(ofColor::black);

	// Add Scenes to the Manager.
	auto sceneManager = entropy::GetSceneManager();
	auto scenePopUps = make_shared<entropy::scene::PopUps>();
	sceneManager->addScene(scenePopUps);
	sceneManager->setCurrentScene(scenePopUps->getName());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	// TODO: Figure out why just letting the destructor do its thing crashes the app.
	entropy::util::App::Destroy();
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
