#ifndef PARTICLE_H
#define PARTICLE_H

#include "ofVectorMath.h"
#include "half.hpp"

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

struct HalfParticle{
	glm::tvec4<half_float::half> pos;
	half_float::half size;
	half_float::half density;
	glm::tvec2<half_float::half> padding;

	HalfParticle(){}

	HalfParticle(const Particle & p)
	:pos(glm::tvec4<half_float::half>(p.pos.x, p.pos.y, p.pos.z, 0))
	,size(p.size)
	,density(p.density){}

	HalfParticle(const glm::vec3 & pos, float size, float density)
	:pos(glm::vec4(pos,0))
	,size(size)
	,density(density){

	}

	inline glm::vec3 getMaxPos() const{
		return pos.xyz() + glm::tvec3<half_float::half>{size*0.5f, size*0.5f, size*0.5f};
	}

	inline glm::vec3 getMinPos() const{
		return pos.xyz() - glm::tvec3<half_float::half>{size*0.5f, size*0.5f, size*0.5f};
	}
};

struct BoundingBox{
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 center;
	glm::vec3 size;

	inline BoundingBox(const glm::vec3 & center, const glm::vec3 & size)
	:min(center - size * 0.5)
	,max(center + size * 0.5)
	,center(center)
	,size(size){}

	BoundingBox(){}

	inline static BoundingBox fromTwoPoints(const glm::vec3 & p1, const glm::vec3 & p2){
		auto max_x = std::max(p1.x, p2.x);
		auto max_y = std::max(p1.y, p2.y);
		auto max_z = std::max(p1.z, p2.z);
		auto min_x = std::min(p1.x, p2.x);
		auto min_y = std::min(p1.y, p2.y);
		auto min_z = std::min(p1.z, p2.z);
		auto max = glm::vec3{max_x, max_y, max_z};
		auto min = glm::vec3{min_x, min_y, min_z};
		auto center = (max + min) * 0.5f;
		auto size = max - min;
		glm::vec3 ret[4] = {min, max, center, size};
		return *reinterpret_cast<BoundingBox*>(&ret);
	}

	inline static BoundingBox fromMinMax(const glm::vec3 & min, const glm::vec3 & max){
		auto center = (max + min) * 0.5f;
		auto size = max - min;
		glm::vec3 ret[4] = {min, max, center, size};
		return *reinterpret_cast<BoundingBox*>(&ret);
	}

	inline bool inside(const glm::vec3 & p) const{
		return p.x>=min.x && p.y>=min.y && p.z>=min.z && p.x<=max.x && p.y<=max.y && p.z<=max.z;
	}

	inline glm::vec3 getSpan() const{
		return max - min;
	}
};
#endif // PARTICLE_H
