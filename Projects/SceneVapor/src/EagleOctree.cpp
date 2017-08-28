#include "EagleOctree.h"
#include "Helpers.h"
#include "ofxEasing.h"
#include "ofCamera.h"

void EagleOctree::load(const ofMesh & vaporOctree){
	auto miny = vaporOctree.getVertices()[0].y;
	auto maxy = vaporOctree.getVertices()[0].y;
	for(auto & v: vaporOctree.getVertices()){
		if(v.y < miny) miny = v.y;
		if(v.y > maxy) maxy = v.y;
	}
	vaporSize = maxy - miny;
	load(vaporSize);
}


void EagleOctree::load(float vaporSize){
	this->vaporSize = vaporSize;
	lastModifed = std::filesystem::last_write_time(ofToDataPath("octree.txt"));
	ofFile octree("octree.txt");
	ofBuffer octreeSrc(octree);

	cubes.clear();

	float miny;
	float maxy;
	bool first = true;

	auto lines = octreeSrc.getLines();

	auto numcubes = 0;
	std::transform(lines.begin(), lines.end(), std::back_inserter(cubes), [&](std::string line){
		float x, y, z, side;
		stringstream sstr;
		sstr << line;
		sstr >> x;
		sstr >> y;
		sstr >> z;
		sstr >> side;
		Cube cube{ {x, -y, z}, side };
		if(first || y < miny) miny = y;
		if(first || y > maxy) maxy = y;
		if(first || side < mincube.side){
			mincube = cube;
		}
		first = false;
		numcubes += 1;
		return cube;
	});
	cout << "numcubes: " << numcubes << endl;

	for(int i=0;i<10;i++){
		mincube.side *= 0.5;
		mincube.pos -= glm::vec3(mincube.side * 0.5);

		cubes.push_back(mincube);

		auto newcube = mincube;
		newcube.pos += glm::vec3{mincube.side, 0, 0};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{0, mincube.side, 0};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{0, 0, mincube.side};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{mincube.side, mincube.side, 0};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{0, mincube.side, mincube.side};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{mincube.side, mincube.side, 0};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{mincube.side, 0, mincube.side};
		cubes.push_back(newcube);

		newcube = mincube;
		newcube.pos += glm::vec3{mincube.side, mincube.side, mincube.side};
		cubes.push_back(newcube);

		mincube = *(cubes.end() - size_t(ofRandom(1,8)));
	}

	maxlevel = 0;
	for(auto & cube: cubes){
		int level = log2(cube.side / mincube.side);
		if(level>maxlevel) maxlevel = level;
	}

	scale = vaporSize / mincube.side;
	boxSize = maxy * scale - miny * scale;
}

void EagleOctree::generateMesh(const ofCamera & camera, glm::vec3 lookAt, ofRectangle viewport){
	auto modified = std::filesystem::last_write_time(ofToDataPath("octree.txt"));
	if(modified > lastModifed){
		cout << "reloading eagle octree" << endl;
		load(vaporSize);
	}
	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_LINES);
	auto cameraDistance = glm::distance(camera.getPosition(), lookAt);
	float alpha;
	glm::quat rot;
	rot = glm::rotate(rot, glm::radians(-90.f), glm::vec3(1,0,0));
	for(auto & cube: cubes){
		auto pos = rot * (cube.pos - mincube.pos) * scale;
		auto size = glm::vec3{cube.side, cube.side, cube.side} * scale;

//		if(cube.pos.z>=mincube.pos.z){
//			auto max = camera.worldToScreen(pos + glm::vec3(cube.side * 0.5 * scale, cube.side * 0.5 * scale, -cube.side * 0.5 * scale));
//			auto min = camera.worldToScreen(pos + glm::vec3(cube.side * 0.5 * scale, -cube.side * 0.5 * scale, -cube.side * 0.5 * scale));
//			float apparentH = glm::distance(max.xy(), min.xy());
//			auto aparentSize = apparentH / viewport.height;
//			alpha = ofMap(aparentSize, 0.8, 0.5, 0, 1, true);
//		}else{
			alpha = ofMap(cameraDistance, cube.side * scale, cube.side * scale * 2, 0, 1, true);
//		}
		mesh.append(entropy::boxWireframe(pos, size, ofFloatColor(1,alpha)));
	}
}

float EagleOctree::getBoxSize() const{
	return boxSize;
}

void EagleOctree::drawMinCube() const{
	auto pos = glm::vec3(0);
	auto size = glm::vec3{mincube.side, mincube.side, mincube.side} * scale;
	entropy::boxWireframe(pos, size, ofFloatColor::white).draw();
}

void EagleOctree::draw() const{
	mesh.draw();
}

glm::vec3 EagleOctree::getPos() const{
	return mincube.pos * scale;
}

void EagleOctree::loadImage(size_t sequence, ofImage & img){
	auto file = "EAGLE OK_" + ofToString(sequence, 5, '0') + ".png";
	img.load(file);
}
