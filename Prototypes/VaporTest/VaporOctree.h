#ifndef VAPOR_OCTREE_H
#define VAPOR_OCTREE_H

#include "ofConstants.h"
#include "ofVectorMath.h"
#include "ofPixels.h"
#include "octree/octree.h"
#include "ofxRange.h"

struct Particle{
	glm::vec3 pos;
	float size;
	float density;

	Particle(){}

	Particle(const glm::vec3 & pos, float size, float density)
	:pos(pos)
	,size(size)
	,density(density){

	}

	inline glm::vec3 getMaxPos() const{
		return pos + glm::vec3{size, size, size};
	}

	inline glm::vec3 getMinPos() const{
		return pos - glm::vec3{size, size, size};
	}
};

class VaporOctree
{
	public:
		void setup(const std::vector<Particle> & particles);
		void compute(size_t resolution, float minDensity, float maxDensity, ofxRange3f coordsRange);
		float getDensity() const;
		bool isLeaf() const;
		void drawLeafs(float minDensity, float maxDensity) const;
		size_t size() const;
		std::vector<Particle> toVector() const;
		ofFloatPixels getPixels(size_t z, float minDensity, float maxDensity) const;
		float getDensity(size_t x, size_t y, size_t z) const;

	private:
		Octree<float,2> * octree = nullptr;
		std::vector<Particle> particles;
};

#endif // OCTREE_H
