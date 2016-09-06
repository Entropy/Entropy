#include "ofApp.h"

#include "entropy/Helpers.h"
#include "entropy/scene/Bubbles.h"
#include "entropy/scene/Calibrate.h"
#include "entropy/scene/Inflation.h"
#include "entropy/scene/Interlude.h"
#include "entropy/scene/Particles.h"
#include "entropy/scene/Surveys.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	ofBackground(ofColor::black);

	// Add all Scenes to the Playlist.
	auto playlist = entropy::GetPlaylist();
	playlist->addScene(make_shared<entropy::scene::Bubbles>());
	playlist->addScene(make_shared<entropy::scene::Calibrate>());
	playlist->addScene(make_shared<entropy::scene::Inflation>());
	playlist->addScene(make_shared<entropy::scene::Interlude>());
	playlist->addScene(make_shared<entropy::scene::Particles>());
	playlist->addScene(make_shared<entropy::scene::Surveys>());
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
