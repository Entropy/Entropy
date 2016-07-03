#ifndef OCTREE_H
#define OCTREE_H

#include "ofConstants.h"
#include "ofVectorMath.h"
#include "ofPixels.h"

struct Particle{
	ofVec3f pos;
	float size;
	float density;

	Particle(){}

	Particle(const ofVec3f & pos, float size, float density)
	:pos(pos)
	,size(size)
	,density(density){

	}

	inline ofVec3f getMaxPos() const{
		return pos + ofVec3f{size, size, size};
	}

	inline ofVec3f getMinPos() const{
		return pos - ofVec3f{size, size, size};
	}
};

struct BoundingBox{
	ofVec3f min;
	ofVec3f max;
	ofVec3f center;
	ofVec3f size;

	inline BoundingBox(const ofVec3f & center, ofVec3f & size)
	:min(center - size * 0.5)
	,max(center + size * 0.5)
	,center(center)
	,size(size){}

	BoundingBox(){}

	inline static BoundingBox fromTwoPoints(const ofVec3f & p1, ofVec3f & p2){
		auto max_x = std::max(p1.x, p2.x);
		auto max_y = std::max(p1.y, p2.y);
		auto max_z = std::max(p1.z, p2.z);
		auto min_x = std::min(p1.x, p2.x);
		auto min_y = std::min(p1.y, p2.y);
		auto min_z = std::min(p1.z, p2.z);
		auto max = ofVec3f{max_x, max_y, max_z};
		auto min = ofVec3f{min_x, min_y, min_z};
		auto center = (max + min) * 0.5f;
		auto size = max - min;
		ofVec3f ret[4] = {min, max, center, size};
		return *reinterpret_cast<BoundingBox*>(&ret);
	}

	inline static BoundingBox fromMinMax(const ofVec3f & min, ofVec3f & max){
		auto center = (max + min) * 0.5f;
		auto size = max - min;
		ofVec3f ret[4] = {min, max, center, size};
		return *reinterpret_cast<BoundingBox*>(&ret);
	}

	inline bool inside(const ofVec3f & p) const{
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