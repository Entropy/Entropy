#ifndef VAPOR_3D_TEXTURE_H
#define VAPOR_3D_TEXTURE_H

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

class Vapor3DTexture
{
	public:
		void setup(const std::vector<Particle> & particles, size_t size, float minDensity, float maxDensity, ofxRange3f coordsRange);
		size_t size() const;
		const std::vector<float> & data() const;
		std::pair<float,float> minmax() const;
	private:
		void add(size_t x, size_t y, size_t z, float value);
		std::vector<float> m_data;
		size_t m_size;
};

#endif // OCTREE_H
