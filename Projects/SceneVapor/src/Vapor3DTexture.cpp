#include "Vapor3DTexture.h"
#include <algorithm>
#include "of3dGraphics.h"
#include "ofGraphics.h"
#include "ofUtils.h"
#include "Constants.h"


namespace ent{
	inline float map(float value, float inMin, float inMax, float outMin, float outMax)
	{
		return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
	}

	inline float zero_map(float value, float inMin, float inMax, float outMax)
	{
		return outMax * (value - inMin) / (inMax - inMin);
	}

	inline float offset_scale(float value, float offset, float scale)
	{
		return (value + offset) * scale;
	}
}

namespace{
	inline size_t idx_clamp(int value, int size){
		return size_t(std::max(std::min(value, size),0));
	}

	struct Box{
		glm::vec3 min;
		glm::vec3 max;
	};

	inline float boxVolume(float size) {
		return size * size * size;
	}

	template<typename B>
	inline float boxesIntersectionVolume(const B & b1, const B & b2){
		return max(min(b1.max.x, b2.max.x) - max(b1.min.x, b2.min.x),0.f)
		* max(min(b1.max.y, b2.max.y) - max(b1.min.y, b2.min.y),0.f)
		* max(min(b1.max.z, b2.max.z) - max(b1.min.z, b2.min.z),0.f);
	}
}

const std::vector<float> & Vapor3DTexture::data() const{
	return m_data;
}

std::vector<float> & Vapor3DTexture::data(){
	return m_data;
}

inline void Vapor3DTexture::add(size_t x, size_t y, size_t z, float value){
	size_t idx = z * this->m_quadsize + y * this->m_size + x;
	m_data[idx] += value;
}

inline size_t Vapor3DTexture::index(size_t x, size_t y, size_t z) const{
	return z * this->m_quadsize + y * this->m_size + x;
}

inline float & Vapor3DTexture::get(size_t idx){
	return m_data[idx];
}

inline void Vapor3DTexture::set(size_t idx, float value){
	m_data[idx] = value;
}

void Vapor3DTexture::setup(const std::vector<Particle> & particles, size_t size, float minDensity, float maxDensity, ofxRange3f coordsRange){
	this->m_size = size;
	this->m_quadsize = size * size;
	this->m_cubesize = size * this->m_quadsize;
	this->m_data.clear();
	this->m_data.assign(size*size*size, 0);
	this->particlesInBox.clear();
	this->groupIndices.clear();
	glm::vec3 coordSpan = coordsRange.getSpan();
	auto normalizeFactor = std::max(std::max(coordSpan.x, coordSpan.y), coordSpan.z);
	auto scale = size / normalizeFactor;
	auto offset = -coordsRange.getMin();
	int isize = int(size);
	for(auto & particle: particles){
		if(!coordsRange.contains(particle.getMaxPos()) ||
		   !coordsRange.contains(particle.getMinPos()) ||
		   (MAX_PARTICLE_SIZE > -1 && particle.size * scale > MAX_PARTICLE_SIZE) ){
			continue;
		}else{
			particlesInBox.push_back(particle);
		}
	}

#if USE_PARTICLES_HISTOGRAM
	ofxRange3f zero;
	zero.clear();
	std::array<float,100> histogram_contribution = {{0}};
	std::array<ofxRange3f,100> histogram_ranges = {{ zero }};
	std::array<float,100> histogram_factor = {{0}};
	std::array<size_t,100> histogram_num_particles = {{0}};
	auto min_max = std::minmax_element(particlesInBox.begin(), particlesInBox.end(), [](const Particle & p1, const Particle & p2){
		return p1.density < p2.density;
	});
	auto absMinDensity = min_max.first->density;
	auto absMaxDensity = min_max.second->density;
	cout << "min density " << absMinDensity << " " << " max " << absMaxDensity << endl;

	float totalDensity = 0.f;
	for(auto & p: particlesInBox){
		auto normalizedDensity = ent::map(p.density, absMinDensity, absMaxDensity, 0, 1);
		auto relDensity = normalizedDensity;///boxVolume(p.size);
		if(relDensity>1){
			cout << "density > 1" << p.density << " min: " << minDensity << " max: " << maxDensity << endl;
			relDensity=1;
		}
		totalDensity += normalizedDensity;
		size_t idx_contrib = size_t(sqrt(sqrt(relDensity)) * 99.f);
		histogram_contribution[idx_contrib] += normalizedDensity;
		histogram_ranges[idx_contrib].add(p.getMaxPos());
		histogram_ranges[idx_contrib].add(p.getMinPos());
		histogram_num_particles[idx_contrib] += 1;
	}

	auto totalSpan = glm::length(coordSpan);
	size_t totalSize = 0;
	for(size_t i=0;i<histogram_ranges.size();++i){
		auto span3f = histogram_ranges[i].getSpan();
		auto span = glm::length(span3f)/totalSpan;
		auto contribution = histogram_contribution[i]/totalDensity;
		if(span>0){
			histogram_factor[i] = contribution / pow(span,3);
		}else if(contribution>0){
			histogram_factor[i] = 1;
		}
		auto binSize = histogram_num_particles[i] * sizeof(Particle);
		totalSize+=binSize;
	}

	size_t i = 0;
	auto total_pct = 0.0;
	cout << "histogram contribution" << endl;
	for(auto & h: histogram_contribution){
		double pct = h/totalDensity;
		total_pct += pct;
		cout << i++ << ": " << pct*100 << "%" << endl;
	}
	cout << "total pct " << total_pct << endl;



	std::array<std::pair<float,size_t>,100> histogram_factor_sorted;
	for(size_t i=0;i<histogram_factor.size();i++){
		histogram_factor_sorted[i] = std::make_pair(histogram_factor[i], histogram_num_particles[i]);
	}
	std::sort(histogram_factor_sorted.begin(), histogram_factor_sorted.end(), [&](const std::pair<float,size_t> & p1, const std::pair<float,size_t> & p2){
		return p1.first > p2.first;
	});
	i = 0;
	total_pct = 0.0;
	double totalFactor = 0;
	size_t byteRate = 27962026; //800Mb/s @ 30fps
	size_t accumulatedSize = 0;
	float threshold=0;
	cout << "histogram factor" << endl;
	for(auto & h: histogram_factor_sorted){
		auto binSize = h.second * sizeof(Particle);
		accumulatedSize+=binSize;
		if(accumulatedSize>byteRate && !(threshold>0)){
			threshold = h.first;
			cout << "found threshold at " << threshold << endl;
		}
		totalFactor+=h.first;
		cout << i++ << ": " << h.first*100 << "% " << binSize/1024. << "KB / " << accumulatedSize/1024./1024. << "MB" << endl;
	}


	particlesInBox.erase(std::remove_if(particlesInBox.begin(), particlesInBox.end(), [&](const Particle & p){
		auto normalizedDensity = ent::map(p.density, absMinDensity, absMaxDensity, 0, 1);
		auto relDensity = normalizedDensity;///boxVolume(p.size);
		if(relDensity>1){
			relDensity=1;
		}
		size_t idx_contrib = size_t(sqrt(sqrt(relDensity)) * 99.f);
		return histogram_factor[idx_contrib] <= threshold;//1.2e-14;
	}), particlesInBox.end());
#endif

#if USE_PARTICLES_COMPUTE_SHADER
	auto total = particlesInBox.size();
	std::vector<Particle> intersectingParticles;
	intersectingParticles.reserve(particlesInBox.size());
	std::swap(intersectingParticles, particlesInBox);
	while(!intersectingParticles.empty()){
		auto nonIntersecting = std::partition(intersectingParticles.begin(), intersectingParticles.end(), [&](const Particle & particle){
			float octree_coords_x = ent::offset_scale(particle.pos.x, offset.x, scale);
			float octree_coords_y = ent::offset_scale(particle.pos.y, offset.y, scale);
			float octree_coords_z = ent::offset_scale(particle.pos.z, offset.z, scale);
			glm::vec3 particlepos = {octree_coords_x, octree_coords_y, octree_coords_z};
			float psize = particle.size * scale;

			if(int(psize)>1){
				Box particleBox = { particlepos - glm::vec3{psize*0.5,psize*0.5,psize*0.5}, particlepos + glm::vec3{psize*0.5,psize*0.5,psize*0.5}};
				float particleVolume = boxVolume(psize);
				auto min_x = idx_clamp(int(octree_coords_x-psize), isize-1);
				auto max_x = idx_clamp(int(octree_coords_x+psize), isize);
				auto min_y = idx_clamp(int(octree_coords_y-psize), isize-1);
				auto max_y = idx_clamp(int(octree_coords_y+psize), isize);
				auto min_z = idx_clamp(int(octree_coords_z-psize), isize-1);
				auto max_z = idx_clamp(int(octree_coords_z+psize), isize);
				for(size_t z = min_z; z<max_z; z++){
					for(size_t y = min_y; y<max_y; y++){
						for(size_t x = min_x; x<max_x; x++){
							auto data_idx = index(x,y,z);
							if(get(data_idx)>0){
								return true;
							}
						}
					}
				}
				for(size_t z = min_z; z<max_z; z++){
					for(size_t y = min_y; y<max_y; y++){
						for(size_t x = min_x; x<max_x; x++){
							auto data_idx = index(x,y,z);
							get(data_idx) += 1.0;
						}
					}
				}
			}else{
				auto data_idx = index(octree_coords_x, octree_coords_y, octree_coords_z);
				if(get(data_idx)>0){
					return true;
				}else{
					get(data_idx) += 1.0;
				}
			}
			return false;
		});
		particlesInBox.insert(particlesInBox.end(), nonIntersecting, intersectingParticles.end());

		cout << "---------------------------------------------------" << endl;
		cout << "intersecting " << nonIntersecting - intersectingParticles.begin() << " non intersecting " << intersectingParticles.end() - nonIntersecting << " intersecting " <<
				float(nonIntersecting - intersectingParticles.begin()) / float(total) << "%" << endl;

		intersectingParticles.erase(nonIntersecting, intersectingParticles.end());
		groupIndices.push_back(particlesInBox.size());

		m_data.assign(m_data.size(), 0.f);
	}

	particlesHalfInBox.resize(particlesInBox.size());
	{
		auto minLimit = std::min(0.f, minDensity);
		auto scale = 1./(maxDensity - minLimit);
		auto offset = -minLimit;
		std::transform(particlesInBox.begin(), particlesInBox.end(), particlesHalfInBox.begin(), [&](Particle & p){
			p.density = ent::offset_scale(p.density, offset, scale);
			return HalfParticle(p);
		});
	}
#endif

#if USE_VOXELS_COMPUTE_SHADER || USE_RAW
	float maxSize = 0;
	float avgSize = 0;
	for(auto & particle: particlesInBox){
		float octree_coords_x = ent::offset_scale(particle.pos.x, offset.x, scale);
		float octree_coords_y = ent::offset_scale(particle.pos.y, offset.y, scale);
		float octree_coords_z = ent::offset_scale(particle.pos.z, offset.z, scale);

		glm::vec3 particlepos = {octree_coords_x, octree_coords_y, octree_coords_z};
		float psize = particle.size * scale;
		maxSize = std::max(maxSize, psize);
		avgSize += psize;

		if(int(psize)>1){
			Box particleBox = { particlepos - glm::vec3{psize*0.5,psize*0.5,psize*0.5}, particlepos + glm::vec3{psize*0.5,psize*0.5,psize*0.5}};
			float particleVolume = boxVolume(psize);
			auto min_x = idx_clamp(int(octree_coords_x-psize), isize-1);
			auto max_x = idx_clamp(int(octree_coords_x+psize), isize);
			auto min_y = idx_clamp(int(octree_coords_y-psize), isize-1);
			auto max_y = idx_clamp(int(octree_coords_y+psize), isize);
			auto min_z = idx_clamp(int(octree_coords_z-psize), isize-1);
			auto max_z = idx_clamp(int(octree_coords_z+psize), isize);
			for(size_t z = min_z; z<max_z; z++){
				for(size_t y = min_y; y<max_y; y++){
					for(size_t x = min_x; x<max_x; x++){
						glm::vec3 voxelpos = {x, y, z};
						Box voxel = {voxelpos, voxelpos + glm::vec3{1.0f, 1.0f, 1.0f}};
						auto factor = boxesIntersectionVolume(voxel, particleBox) / particleVolume;
						this->add(x, y, z, factor * particle.density);
					}
				}
			}
		}else{
			this->add(octree_coords_x, octree_coords_y, octree_coords_z, particle.density);
		}
	}
	avgSize /= particlesInBox.size();

	cout << "max particle size " << maxSize << endl;
	cout << "avg particle size " << avgSize << endl;


	/*{
		auto minLimit = std::min(0.f, minDensity);
		auto scale = 1./(maxDensity - minLimit);
		auto offset = -minLimit;
		std::transform(this->m_data.begin(), this->m_data.end(), this->m_data.begin(), [&](float v){
			return ent::offset_scale(v, offset, scale);
		});
	}*/
#endif
}

size_t Vapor3DTexture::size() const{
	return m_size;
}

std::pair<float,float> Vapor3DTexture::minmax() const{
	auto minmax = std::minmax_element(this->m_data.begin(), this->m_data.end());
	return std::make_pair(*minmax.first, *minmax.second);
}

const std::vector<Particle> & Vapor3DTexture::getParticlesInBox() const{
	return particlesInBox;
}

const std::vector<size_t> & Vapor3DTexture::getGroupIndices() const{
	return groupIndices;
}

const std::vector<HalfParticle> & Vapor3DTexture::getHalfParticlesInBox() const{
	return particlesHalfInBox;
}
