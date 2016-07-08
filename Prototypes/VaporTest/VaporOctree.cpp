#include "VaporOctree.h"
#include <algorithm>
#include "of3dGraphics.h"
#include "ofGraphics.h"
#include "ofUtils.h"
#include "ThreadPool.h"
#include <future>


namespace{
	struct Box{
		glm::vec3 min;
		glm::vec3 max;
	};

	inline float boxVolume(float size) {
		return size * size * size;
	}

	inline float boxesIntersectionVolume(const Box & b1, const Box & b2){
		return max(min(b1.max.x, b2.max.x) - max(b1.min.x, b2.min.x),0.f)
		* max(min(b1.max.y, b2.max.y) - max(b1.min.y, b2.min.y),0.f)
		* max(min(b1.max.z, b2.max.z) - max(b1.min.z, b2.min.z),0.f);
	}
}
void VaporOctree::setup(const std::vector<Particle> & particles){
	this->particles = particles;
}


void VaporOctree::compute(size_t resolution, float minDensity, float maxDensity, ofxRange3f coordsRange){
	this->octree = new Octree<float,2>(resolution);
	glm::vec3 coordSpan = coordsRange.getSpan();
	auto originShift = -0.5f * coordSpan - coordsRange.getMin();
	auto normalizeFactor = std::max(std::max(coordSpan.x, coordSpan.y), coordSpan.z);
	auto scale = resolution / normalizeFactor;
	for(auto & particle: this->particles){
		float octree_coords_x = ofMap(particle.pos.x, coordsRange.getMin().x, coordsRange.getMax().x, 0, resolution);
		float octree_coords_y = ofMap(particle.pos.y, coordsRange.getMin().y, coordsRange.getMax().y, 0, resolution);
		float octree_coords_z = ofMap(particle.pos.z, coordsRange.getMin().z, coordsRange.getMax().z, 0, resolution);
		glm::vec3 particlepos = {octree_coords_x, octree_coords_y, octree_coords_z};
		float size = particle.size * scale;
		//auto density = this->octree->at(octree_coords_x, octree_coords_y, octree_coords_z);
		//this->octree->set(octree_coords_x, octree_coords_y, octree_coords_z, density + particle.density);
		Box particleBox = { particlepos - glm::vec3{size,size,size}, particlepos + glm::vec3{size,size,size}};
		for(int z = -size; z<size; z++){
			for(int y = -size; y<size; y++){
				for(int x = -size; x<size; x++){
					int off_x = octree_coords_x + x;
					int off_y = octree_coords_y + y;
					int off_z = octree_coords_z + z;
					glm::vec3 voxelpos = {off_x, off_y, off_z};
					Box voxel = {voxelpos, voxelpos + glm::vec3{1.0f, 1.0f, 1.0f}};
					auto factor = boxesIntersectionVolume(voxel, particleBox);
					//cout << factor << endl;
					auto density = this->octree->at(off_x, off_y, off_z);
					this->octree->set(off_x, off_y, off_z, density + particle.density * factor);
				}
			}
		}
	}
}


bool VaporOctree::isLeaf() const{
	return false;
}

void VaporOctree::drawLeafs(float minDensity, float maxDensity) const{

}

size_t VaporOctree::size() const{
	return octree->size();
}

std::vector<Particle> VaporOctree::toVector() const{
	/*std::vector<Particle> particles;
	for(size_t z=0, i=0; z<octree->size();z++){
		auto array2d = this->octree->zSlice(z);
		for(size_t y=0; y<octree->size();++y){
			for(size_t x=0; x<octree->size();++x,++i){
				auto density = array2d.at(x,y);
				if(density>minDensity && density<maxDensity){
					particles.push_back({{x + 0.5, y + 0.5, z + 0.5}, 1.0, density});
				}
			}
		}
	}
	return particles;*/
}

ofFloatPixels VaporOctree::getPixels(size_t z, float minDensity, float maxDensity) const{
	ofFloatPixels pixels;
	pixels.allocate(octree->size(), octree->size(), OF_PIXELS_GRAY);
	auto array2d = this->octree->zSlice(z);
	for(size_t y=0, i=0; y<octree->size();++y){
		for(size_t x=0; x<octree->size();++x,++i){
			auto density = array2d.at(x,y);
			pixels[i] = ofMap(density, minDensity, maxDensity, 0.0, 1.0);
		}
	}
	return pixels;
}


float VaporOctree::getDensity(size_t x, size_t y, size_t z) const{
	return octree->at(x,y,z);
}
