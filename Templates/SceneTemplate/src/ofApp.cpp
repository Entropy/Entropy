#include "ofApp.h"

#include "entropy/Helpers.h"
#include "entropy/scene/Template.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(ofColor::black);

	// Add Scenes to Manager.
	auto sceneManager = entropy::GetSceneManager();
	auto sceneTemplate = make_shared<entropy::scene::Template>();
	sceneManager->addScene(sceneTemplate);
	sceneManager->setCurrentScene(sceneTemplate->getName());

	// Make the Canvas fill the window when it's resized.
	auto canvas = entropy::GetCanvas();
	canvas->setFillWindow(true);
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
