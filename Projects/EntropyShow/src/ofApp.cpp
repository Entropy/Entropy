#include "ofApp.h"
#include "ofxHPVPlayer.h"

#include "entropy/Helpers.h"
#include "entropy/scene/Calibrate.h"
#include "entropy/scene/Empty.h"
#include "entropy/scene/Interlude.h"
#include "entropy/util/App.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofSetLogLevel(OF_LOG_NOTICE);
	ofBackground(ofColor::black);

	// Start the HPV engine.
	HPV::InitHPVEngine();

	// Add all Scenes to the Playlist.
	auto playlist = entropy::GetPlaylist();
	playlist->addScene(std::make_shared<entropy::scene::Calibrate>());
    playlist->addScene(std::make_shared<entropy::scene::Empty>());
//	playlist->addScene(std::make_shared<entropy::scene::Interlude>());
}

//--------------------------------------------------------------
void ofApp::exit()
{
	// Stop the HPV engine.
	HPV::DestroyHPVEngine();
	
	// TODO: Figure out why just letting the destructor do its thing crashes the app.
	entropy::util::App::Destroy();
}

//--------------------------------------------------------------
void ofApp::update()
{
	HPV::Update();
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
