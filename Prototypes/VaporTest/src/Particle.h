#ifndef PARTICLE_H
#define PARTICLE_H

#include "ofVectorMath.h"

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
		return pos + glm::vec3{size*0.5f, size*0.5f, size*0.5f};
	}

	inline glm::vec3 getMinPos() const{
		return pos - glm::vec3{size*0.5f, size*0.5f, size*0.5f};
	}
};
#endif // PARTICLE_H
