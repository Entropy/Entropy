#include "ofApp.h"
#include "Helpers.h"


ofMesh boxWireframe(glm::vec3 pos, glm::vec3 size, const ofFloatColor & color){
	ofMesh boxWireframeMesh;
	boxWireframeMesh.setMode( OF_PRIMITIVE_LINES );

	boxWireframeMesh.addVertex(glm::vec3{-.5f, -.5f, -.5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{.5f, -.5f, -.5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{.5f, .5f, -.5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{-.5f, .5f, -.5f} * size + pos);

	boxWireframeMesh.addVertex(glm::vec3{-.5f, -.5f, .5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{.5f, -.5f, .5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{.5f, .5f, .5f} * size + pos);
	boxWireframeMesh.addVertex(glm::vec3{-.5f, .5f, .5f} * size + pos);

	boxWireframeMesh.getColors().assign(8, color);

	// front face
	boxWireframeMesh.addIndex(0);
	boxWireframeMesh.addIndex(1);

	boxWireframeMesh.addIndex(1);
	boxWireframeMesh.addIndex(2);

	boxWireframeMesh.addIndex(2);
	boxWireframeMesh.addIndex(3);

	boxWireframeMesh.addIndex(3);
	boxWireframeMesh.addIndex(0);

	// back face
	boxWireframeMesh.addIndex(4);
	boxWireframeMesh.addIndex(5);

	boxWireframeMesh.addIndex(5);
	boxWireframeMesh.addIndex(6);

	boxWireframeMesh.addIndex(6);
	boxWireframeMesh.addIndex(7);

	boxWireframeMesh.addIndex(7);
	boxWireframeMesh.addIndex(4);


	boxWireframeMesh.addIndex(0);
	boxWireframeMesh.addIndex(4);

	boxWireframeMesh.addIndex(1);
	boxWireframeMesh.addIndex(5);

	boxWireframeMesh.addIndex(2);
	boxWireframeMesh.addIndex(6);

	boxWireframeMesh.addIndex(3);
	boxWireframeMesh.addIndex(7);

	return boxWireframeMesh;
}

//--------------------------------------------------------------
void ofApp::setup(){
	image.load("EAGLE OK_00027.png");

	reloadOctree();

	auto setFov = [this](float &){
		camera.setFov(fov);

		float halfFov = PI * fov / 360.;
		float halfSize = boxSize / (2. * cameraMagnification);
		float distance = halfSize/tan(halfFov);

		camera.setDistance(distance);
		camera.setNearClip(std::max(distance-boxSize, 0.1f));
		camera.setFarClip(distance + boxSize);
	};
	auto f = camera.getFov();
	setFov(f);
	fov.ownListener(setFov);
	cameraMagnification.ownListener(setFov);

	crop.ownListener([this](float & crop){
		auto w = ofGetWidth() + crop;
		auto h = w * image.getHeight() / double(image.getWidth());
		ofSetWindowShape(ofGetWidth(), h);
	});

	fbo.allocate(entropy::GetSceneWidth(), entropy::GetSceneHeight(), GL_RGBA);

	rotation.ownListener([this](float & rotation){
		camera.orbitDeg(0,rotation,camera.getDistance());
	});
}

//--------------------------------------------------------------
void ofApp::reloadOctree(){
	lastModifed = std::filesystem::last_write_time(ofToDataPath("octree.txt"));
	ofFile octree("octree.txt");
	octree >> octreeSrc;
	mesh.setMode(OF_PRIMITIVE_LINES);

	auto scale = 1.f;
	mesh.clear();
	float miny = std::numeric_limits<float>::max();
	float maxy = std::numeric_limits<float>::min();
	auto numcubes = 0;
	for(auto & line: octreeSrc.getLines()){
		float x, y, z, side;
		stringstream sstr;
		sstr << line;
		sstr >> x;
		sstr >> y;
		sstr >> z;
		sstr >> side;
		if(y < miny) miny = y;
		if(y > maxy) maxy = y;
		glm::vec3 pos{x * scale, -y * scale, z*scale};
		glm::vec3 size{side * scale, side * scale, side * scale};
		numcubes += 1;
		mesh.append(boxWireframe(pos, size, ofFloatColor::white));
	}
	boxSize = maxy - miny;
	cout << numcubes << endl;
}

//--------------------------------------------------------------
void ofApp::update(){
	auto modified = std::filesystem::last_write_time(ofToDataPath("octree.txt"));
	if(modified > lastModifed){
		cout << "reloading" << endl;
		reloadOctree();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo.begin();
	ofClear(0,255);
	auto w = fbo.getWidth() + crop;
	auto h = w * image.getHeight() / (image.getWidth());
	image.draw((fbo.getWidth() - w) / 2,(fbo.getHeight() - h)/2,w,h);

	camera.begin();
	mesh.draw();
	camera.end();
	fbo.end();

	w = ofGetWidth();
	h = w * fbo.getHeight() / fbo.getWidth();
	fbo.draw(0,(ofGetHeight() - h) / 2,w,h);

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
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY ){
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
