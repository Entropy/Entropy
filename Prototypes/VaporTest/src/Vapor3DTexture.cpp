#include "Vapor3DTexture.h"
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

namespace{
inline size_t idx_clamp(int value, int size){
	return size_t(std::max(std::min(value, size),0));
}
}

const std::vector<float> & Vapor3DTexture::data() const{
	return m_data;
}

void Vapor3DTexture::add(size_t x, size_t y, size_t z, float value){
	size_t idx = z * m_size * m_size + y * m_size + x;
	m_data[idx] += value;
}

void Vapor3DTexture::setup(const std::vector<Particle> & particles, size_t size, float minDensity, float maxDensity, ofxRange3f coordsRange){
	this->m_size = size;
	this->m_data.clear();
	this->m_data.assign(size*size*size, 0);
	glm::vec3 coordSpan = coordsRange.getSpan();
	auto normalizeFactor = std::max(std::max(coordSpan.x, coordSpan.y), coordSpan.z);
	auto scale = size / normalizeFactor;
	int isize = int(size);
	for(auto & particle: particles){
		float octree_coords_x = ofMap(particle.pos.x, coordsRange.getMin().x, coordsRange.getMax().x, 0, size);
		float octree_coords_y = ofMap(particle.pos.y, coordsRange.getMin().y, coordsRange.getMax().y, 0, size);
		float octree_coords_z = ofMap(particle.pos.z, coordsRange.getMin().z, coordsRange.getMax().z, 0, size);
		glm::vec3 particlepos = {octree_coords_x, octree_coords_y, octree_coords_z};
		float psize = particle.size * scale;

		Box particleBox = { particlepos - glm::vec3{psize*0.5,psize*0.5,psize*0.5}, particlepos + glm::vec3{psize*0.5,psize*0.5,psize*0.5}};
		float particleVolume = boxVolume(psize);
		for(int z = -psize; z<psize; z++){
			for(int y = -psize; y<psize; y++){
				for(int x = -psize; x<psize; x++){
					auto off_x = idx_clamp(int(octree_coords_x) + x, isize);
					auto off_y = idx_clamp(int(octree_coords_y) + y, isize);
					auto off_z = idx_clamp(int(octree_coords_z) + z, isize);
					glm::vec3 voxelpos = {off_x, off_y, off_z};
					Box voxel = {voxelpos, voxelpos + glm::vec3{1.0f, 1.0f, 1.0f}};
					auto factor = boxesIntersectionVolume(voxel, particleBox) / particleVolume;
					this->add(off_x, off_y, off_z, particle.density * factor);
				}
			}
		}
	}

	std::transform(this->m_data.begin(), this->m_data.end(), this->m_data.begin(), [&](float v){
		return ofMap(v, minDensity, maxDensity, 0, 1);
	});
}

size_t Vapor3DTexture::size() const{
	return m_size;
}

std::pair<float,float> Vapor3DTexture::minmax() const{
	auto minmax = std::minmax_element(this->m_data.begin(), this->m_data.end());
	return std::make_pair(*minmax.first, *minmax.second);
}
