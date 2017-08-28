#ifndef EAGLEOCTREE_H
#define EAGLEOCTREE_H

#include "ofConstants.h"
#include "ofVboMesh.h"
#include "ofImage.h"

class EagleOctree
{
public:
	void load(const ofMesh & vaporOctree);
	void load(float vaporSize);
	void generateMesh(const ofCamera & camera, glm::vec3 lookAt, ofRectangle viewport);
	float getBoxSize() const;
	glm::vec3 getPos() const;
	void draw() const;
	void drawMinCube() const;
	void loadImage(size_t sequence, ofImage & image);
private:
	struct Cube{
		glm::vec3 pos;
		float side;
	};
	std::vector<Cube> cubes;
	ofVboMesh mesh;
	float boxSize = 0;
	std::filesystem::file_time_type lastModifed;
	float scale = 1;
	Cube mincube;
	int maxlevel = 0;
	float vaporSize;
};

#endif // EAGLEOCTREE_H
