#ifndef PARTICLE_H
#define PARTICLE_H

#include "ofVectorMath.h"
#include "fbi/tuplegenerator.h" //TraitsGenerator
#include "fbi/tuple.h"
#include "fbi/fbi.h" //SetA::intersect

struct Particle{
	glm::vec4 pos;
	float size;
	float density;
	glm::vec2 padding;

	Particle(){}

	Particle(const glm::vec3 & pos, float size, float density)
	:pos(glm::vec4(pos,0))
	,size(size)
	,density(density){

	}

	inline glm::vec3 getMaxPos() const{
		return pos.xyz() + glm::vec3{size*0.5f, size*0.5f, size*0.5f};
	}

	inline glm::vec3 getMinPos() const{
		return pos.xyz() - glm::vec3{size*0.5f, size*0.5f, size*0.5f};
	}
};

#endif // PARTICLE_H
