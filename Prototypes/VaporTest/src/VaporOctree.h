#ifndef VAPOR_OCTREE_H
#define VAPOR_OCTREE_H

#include "ofConstants.h"
#include "ofVectorMath.h"
#include "ofPixels.h"
#include "Particle.h"


struct BoundingBox{
	glm::vec3 min;
	glm::vec3 max;
	glm::vec3 center;
	glm::vec3 size;

	inline BoundingBox(const glm::vec3 & center, glm::vec3 & size)
	:min(center - size * 0.5)
	,max(center + size * 0.5)
	,center(center)
	,size(size){}

	BoundingBox(){}

	inline static BoundingBox fromTwoPoints(const glm::vec3 & p1, glm::vec3 & p2){
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
};

class VaporOctree
{
	public:
		VaporOctree();
		void setup(const std::vector<Particle> & particles);
		void compute(size_t resolution, float minDensity, float maxDensity);
		float getDensity() const;
		bool isLeaf() const;
		void drawLeafs(float minDensity, float maxDensity) const;
		size_t getMaxLevel() const;
		size_t size() const;
		std::vector<Particle> toVector() const;
		ofFloatPixels getPixels(size_t z, float minDensity, float maxDensity) const;
		float getDensity(size_t x, size_t y, size_t z) const;

	private:
		void compute(size_t resolution, float minDensity, float maxDensity, size_t level);
		bool divide(size_t resolution, float minDensity, float maxDensity);
		size_t getMaxLevel(size_t current) const;
		std::shared_ptr<std::vector<Particle>> particles;
		std::vector<size_t> particlesIndex;
		BoundingBox bb;
		float density;
		std::vector<VaporOctree> children;
		size_t resolution;
		size_t x,y,z;
};

#endif // OCTREE_H
