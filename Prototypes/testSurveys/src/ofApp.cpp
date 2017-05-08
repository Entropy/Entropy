#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	std::vector<ofFloatColor> colors{
		ofFloatColor::white,
		ofFloatColor::darkBlue,
		ofFloatColor::white,
		ofFloatColor::cyan,
		ofFloatColor::lightBlue,
		ofFloatColor::aliceBlue,
	};
	galaxy.load("galaxy.ply");
	galaxy.setMode(OF_PRIMITIVE_POINTS);

	for(auto & v: galaxy.getVertices()){
		auto d = glm::length2(v);
		v.y *= 0.8;
		auto pct = ofMap(d, 0, 0.85, 1, 0, true);
		auto bri = ofRandom(1) > 0.99 ? 35 : 15;
		galaxy.addColor(colors[int(ofRandom(colors.size()))] * bri * pct);
	}

	camera.setDistance(5);
	camera.setNearClip(0.01);

	ofSetBackgroundColor(0);

	ofFbo::Settings settings;
	settings.width = ofGetWidth();
	settings.height = ofGetHeight();
	settings.internalformat = GL_RGBA32F;
	settings.textureTarget = GL_TEXTURE_2D;
	settings.numSamples = 4;
	fbo1.allocate(settings);
	fbo2.allocate(settings);

	postEffects.resize(ofGetWidth(), ofGetHeight());
	renderer.setup(1);
	renderer.resize(ofGetWidth(), ofGetHeight());

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo1.begin();
	ofClear(0,255);
	camera.begin();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	renderer.draw(galaxy.getVbo(), 0, galaxy.getNumVertices(), GL_POINTS, camera);
	//ofDrawBox(1);
	camera.end();
	fbo1.end();

	postEffects.process(fbo1.getTexture(), fbo2, postParameters);

	ofDisableAlphaBlending();
	ofSetColor(255);
	fbo2.draw(0,0);

	ofEnableAlphaBlending();
	gui.draw();
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
