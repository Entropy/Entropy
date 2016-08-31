#include "ofApp.h"
#include "ofGstVideoPlayer.h"

//--------------------------------------------------------------
void ofApp::setup(){
	auto player = std::make_shared<ofGstVideoPlayer>();
	player1.setPixelFormat(OF_PIXELS_NATIVE);
	player1.setPlayer(player);

	player = std::make_shared<ofGstVideoPlayer>();
	player2.setPixelFormat(OF_PIXELS_NATIVE);
	player2.setPlayer(player);

	player1.load("Entropy_Evolve.mp4");
	player2.load("Entropy_Evolve.mp4");

	player1.play();
	player2.play();
}

//--------------------------------------------------------------
void ofApp::update(){
	player1.update();
	player2.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
	auto ratio = player1.getWidth() / player1.getHeight();
	auto w = ofGetWidth();
	auto h = w / ratio;
	player1.draw(0, 0, w, h);
	player2.draw(0, h, w, h);
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
