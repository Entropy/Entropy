#ifndef VAPOR_OCTREE_H
#define VAPOR_OCTREE_H

#include "ofConstants.h"
#include "ofVectorMath.h"
#include "ofPixels.h"
#include "Particle.h"
#include "ofMesh.h"


class VaporOctree
{
	public:
		VaporOctree();
		void setup(const std::vector<Particle> & particles);
		void compute(size_t resolution, float minDensity, float maxDensity);
		float getDensity() const;
		bool isLeaf() const;
		void drawLeafs(float minDensity, float maxDensity) const;
		void drawDensities(const ofTrueTypeFont & ttf, const ofCamera & camera, const glm::mat4 & model, float minDensity, float maxDensity) const;
		size_t getMaxLevel() const;
		size_t size() const;
		std::vector<Particle> toVector() const;
		ofFloatPixels getPixels(size_t z, float minDensity, float maxDensity) const;
		float getDensity(size_t x, size_t y, size_t z) const;

		enum MeshSort{
			SizeLargerFirst,
			SizeSmallerFirst,
			DensityLargerFirst,
			DensitySmallerFirst
		};

		ofMesh getMesh(float minDensity, float maxDensity, MeshSort meshsort, int minLevel) const;

	private:
		void compute(size_t resolution, float minDensity, float maxDensity, size_t level);
		bool divide(size_t resolution, float minDensity, float maxDensity);
		void getChildrenRecursively(std::vector<const VaporOctree*> & childrenPtr) const;
		size_t getMaxLevel(size_t current) const;
		std::shared_ptr<std::vector<Particle>> particles;
		std::vector<size_t> particlesIndex;
		BoundingBox bb;
		float density;
		std::vector<VaporOctree> children;
		size_t resolution;
		size_t x,y,z;
		size_t level = 0;
		bool firstFrame = true;
};

#endif // OCTREE_H
