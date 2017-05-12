#include "ofApp.h"
#include "entropy/geom/ofxPlane.h"

std::array<ofxPlane,6> frustum(const ofCamera & camera){
	auto comboMatrix = camera.getModelViewProjectionMatrix();

	std::array<ofxPlane,6> p_planes;
	p_planes[0].a = comboMatrix[3][0] + comboMatrix[0][0];
	p_planes[0].b = comboMatrix[3][1] + comboMatrix[0][1];
	p_planes[0].c = comboMatrix[3][2] + comboMatrix[0][2];
	p_planes[0].d = comboMatrix[3][3] + comboMatrix[0][3];
	// Right clipping plane
	p_planes[1].a = comboMatrix[3][0] - comboMatrix[0][0];
	p_planes[1].b = comboMatrix[3][1] - comboMatrix[0][1];
	p_planes[1].c = comboMatrix[3][2] - comboMatrix[0][2];
	p_planes[1].d = comboMatrix[3][3] - comboMatrix[0][3];
	// Top clipping plane
	p_planes[2].a = comboMatrix[3][0] - comboMatrix[1][0];
	p_planes[2].b = comboMatrix[3][1] - comboMatrix[1][1];
	p_planes[2].c = comboMatrix[3][2] - comboMatrix[1][2];
	p_planes[2].d = comboMatrix[3][3] - comboMatrix[1][3];
	// Bottom clipping plane
	p_planes[3].a = comboMatrix[3][0] + comboMatrix[1][0];
	p_planes[3].b = comboMatrix[3][1] + comboMatrix[1][1];
	p_planes[3].c = comboMatrix[3][2] + comboMatrix[1][2];
	p_planes[3].d = comboMatrix[3][3] + comboMatrix[1][3];
	// Near clipping plane
	p_planes[4].a = comboMatrix[3][0] + comboMatrix[2][0];
	p_planes[4].b = comboMatrix[3][1] + comboMatrix[2][1];
	p_planes[4].c = comboMatrix[3][2] + comboMatrix[2][2];
	p_planes[4].d = comboMatrix[3][3] + comboMatrix[2][3];
	// Far clipping plane
	p_planes[5].a = comboMatrix[3][0] - comboMatrix[2][0];
	p_planes[5].b = comboMatrix[3][1] - comboMatrix[2][1];
	p_planes[5].c = comboMatrix[3][2] - comboMatrix[2][2];
	p_planes[5].d = comboMatrix[3][3] - comboMatrix[2][3];

	for(auto & p: p_planes){
		p = p.normalized();
	}

	return p_planes;
}


bool pointInFrustum(const std::array<ofxPlane,6> & frustum, glm::vec3 p){
	for(auto & plane: frustum){
		if(plane.distanceToPoint(p)<0){
			return false;
		}
	}
	return true;
}

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

	glm::vec3 bmin = galaxy.getVertices()[0], bmax = galaxy.getVertices()[0];
	for(auto & v: galaxy.getVertices()){
		auto d = glm::length2(v);
		v.y *= 0.8;
		auto pct = ofMap(d, 0, 0.85, 1, 0, true);
		auto bri = ofRandom(1) > 0.99 ? 35 : 15;
		galaxy.addColor(colors[int(ofRandom(colors.size()))] * bri * pct);
		if(v.x<bmin.x) bmin.x = v.x;
		if(v.y<bmin.y) bmin.y = v.y;
		if(v.z<bmin.z) bmin.z = v.z;
		if(v.x>bmax.x) bmax.x = v.x;
		if(v.y>bmax.y) bmax.y = v.y;
		if(v.z>bmax.z) bmax.z = v.z;
	}

	cout << bmin << " - " << bmax << endl;

	camera.setDistance(5);
	camera.setNearClip(0.01);
	camera.setRelativeYAxis(true);

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

	ofxTextureRecorder::Settings texSettings(fbo1.getTexture());
	texSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
	recorder.setup(texSettings);

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
//	auto far = camera.getFarClip();
//	camera.setFarClip(std::max(10.*ofGetMouseX()/float(ofGetWidth()), 0.01));

//	auto p = camera.worldToScreen(glm::vec3(0));
//	cout << p.z << endl;
//	if(p.z>=0. && p.z<=1. && p.x>0 && p.x<ofGetWidth() && p.y>0 && p.y<ofGetHeight()){
		renderer.draw(galaxy.getVbo(), 0, galaxy.getNumVertices(), GL_POINTS, camera);
//	}else{
//		ofDrawSphere(glm::vec3(0), 1);
//	}
//	camera.setFarClip(far);
	//ofDrawBox(1);
	camera.end();
	fbo1.end();

	postEffects.process(fbo1.getTexture(), fbo2, postParameters);

	ofDisableAlphaBlending();
	ofSetColor(255);
	fbo2.draw(0,0);

	if(save){
		recorder.save(fbo2.getTexture());
	}

	ofEnableAlphaBlending();
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key=='s'){
		save = !save;
	}
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
