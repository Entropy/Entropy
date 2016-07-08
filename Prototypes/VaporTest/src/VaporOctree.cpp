#include "VaporOctree.h"
#include <algorithm>
#include "of3dGraphics.h"
#include "ofGraphics.h"
#include "ofUtils.h"
#include <future>

struct BoundingBoxSearch {
	inline BoundingBoxSearch(const ofVec3f & min, const ofVec3f & max, float density)
	:min(min)
	,max(max)
	,density(density){}

	BoundingBoxSearch(){}

	ofVec3f min = {std::numeric_limits <float>::max(), std::numeric_limits <float>::max(), std::numeric_limits <float>::max()};
	ofVec3f max = {std::numeric_limits <float>::lowest(), std::numeric_limits <float>::lowest(), std::numeric_limits <float>::lowest()};
	float density = 0.0f;
};

VaporOctree::VaporOctree()
:particles(new std::vector <Particle>())
,density(0.f)
,x(0)
,y(0)
,z(0){}

void VaporOctree::setup(const std::vector <Particle> & particles){
	*this->particles = particles;
	this->particlesIndex.resize(this->particles->size());
	std::iota(this->particlesIndex.begin(), this->particlesIndex.end(), 0);
	auto bb = std::accumulate(particles.begin(), particles.end(), BoundingBoxSearch(), [] (const BoundingBoxSearch &box, const Particle &p)->BoundingBoxSearch {
		auto maxpos = p.getMaxPos();
		auto minpos = p.getMinPos();
		auto max_x = std::max(maxpos.x, box.max.x);
		auto max_y = std::max(maxpos.y, box.max.y);
		auto max_z = std::max(maxpos.z, box.max.z);
		auto min_x = std::min(minpos.x, box.min.x);
		auto min_y = std::min(minpos.y, box.min.y);
		auto min_z = std::min(minpos.z, box.min.z);
		auto density = box.density + p.density;
		return {{min_x, min_y, min_z}, {max_x, max_y, max_z}, density};
	});
	this->bb = BoundingBox::fromMinMax(bb.min,  bb.max);
	this->density = bb.density;
}


bool VaporOctree::divide(size_t resolution, float minDensity, float maxDensity){
	float span = maxDensity - minDensity;
	float thresDensity = minDensity + span * 0.002f;
	if(density < thresDensity){
		particlesIndex.clear();
		density = 0;
	}
	if(resolution == 0 || particlesIndex.size() == 0){
		return false;
	}
	auto bbmin = this->bb.min;
	auto bbmax = this->bb.max;
	glm::vec3 p1 = bbmin;
	glm::vec3 p2 = {bbmin.x, bbmin.y, bbmax.z};
	glm::vec3 p3 = {bbmin.x, bbmax.y, bbmin.z};
	glm::vec3 p4 = {bbmin.x, bbmax.y, bbmax.z};
	glm::vec3 p5 = {bbmax.x, bbmin.y, bbmin.z};
	glm::vec3 p6 = {bbmax.x, bbmin.y, bbmax.z};
	glm::vec3 p7 = {bbmax.x, bbmax.y, bbmin.z};
	glm::vec3 p8 = bbmax;

	glm::vec3 center = (p1 + p8) * 0.5;

	size_t incr = pow(2, resolution) / 4.;
	children.resize(8);
	children[0].bb = BoundingBox::fromTwoPoints(p1, center);
	children[0].particles = this->particles;
	children[0].x = this->x - incr;
	children[0].y = this->y - incr;
	children[0].z = this->z - incr;
	children[1].bb = BoundingBox::fromTwoPoints(p2, center);
	children[1].particles = this->particles;
	children[1].x = this->x - incr;
	children[1].y = this->y - incr;
	children[1].z = this->z + incr;
	children[2].bb = BoundingBox::fromTwoPoints(p3, center);
	children[2].particles = this->particles;
	children[2].x = this->x - incr;
	children[2].y = this->y + incr;
	children[2].z = this->z - incr;
	children[3].bb = BoundingBox::fromTwoPoints(p4, center);
	children[3].particles = this->particles;
	children[3].x = this->x - incr;
	children[3].y = this->y + incr;
	children[3].z = this->z + incr;
	children[4].bb = BoundingBox::fromTwoPoints(p5, center);
	children[4].particles = this->particles;
	children[4].x = this->x + incr;
	children[4].y = this->y - incr;
	children[4].z = this->z - incr;
	children[5].bb = BoundingBox::fromTwoPoints(p6, center);
	children[5].particles = this->particles;
	children[5].x = this->x + incr;
	children[5].y = this->y - incr;
	children[5].z = this->z + incr;
	children[6].bb = BoundingBox::fromTwoPoints(p7, center);
	children[6].particles = this->particles;
	children[6].x = this->x + incr;
	children[6].y = this->y + incr;
	children[6].z = this->z - incr;
	children[7].bb = BoundingBox::fromTwoPoints(p8, center);
	children[7].particles = this->particles;
	children[7].x = this->x + incr;
	children[7].y = this->y + incr;
	children[7].z = this->z + incr;

	std::array <float, 8> distances {{ 0.f }};

	for(auto i : particlesIndex){
		auto & p = (*particles)[i];
		distances[0] = glm::dot(children[0].bb.center - p.pos, children[0].bb.center - p.pos);
		distances[1] = glm::dot(children[1].bb.center - p.pos, children[1].bb.center - p.pos);
		distances[2] = glm::dot(children[2].bb.center - p.pos, children[2].bb.center - p.pos);
		distances[3] = glm::dot(children[3].bb.center - p.pos, children[3].bb.center - p.pos);
		distances[4] = glm::dot(children[4].bb.center - p.pos, children[4].bb.center - p.pos);
		distances[5] = glm::dot(children[5].bb.center - p.pos, children[5].bb.center - p.pos);
		distances[6] = glm::dot(children[6].bb.center - p.pos, children[6].bb.center - p.pos);
		distances[7] = glm::dot(children[7].bb.center - p.pos, children[7].bb.center - p.pos);
		auto min = std::min_element(distances.begin(), distances.end()) - distances.begin();
		children[min].particlesIndex.push_back(i);
		children[min].density += p.density;
	}

	return true;
}

void VaporOctree::compute(size_t resolution, float minDensity, float maxDensity){
	this->resolution = resolution;
	size_t center = pow(2, resolution) / 2.;
	this->x = center;
	this->y = center;
	this->z = center;
	if(this->divide(resolution, minDensity, maxDensity)){
		std::array <std::future <bool>, 8> futures;
		for(size_t i = 0; i < children.size(); ++i){
			auto & child = children[i];
			futures[i] = std::async(std::launch::async, [&child, resolution, minDensity, maxDensity] {
				child.compute(resolution - 1, minDensity, maxDensity, 0);
				return true;
			});
		}

		for(auto & future : futures){
			future.wait();
		}
	}
}

void VaporOctree::compute(size_t resolution, float minDensity, float maxDensity, size_t level){
	this->resolution = resolution;
	if(this->divide(resolution, minDensity, maxDensity)){
		for(auto & child : children){
			child.compute(resolution - 1, minDensity, maxDensity, level + 1);
		}
	}
}


bool VaporOctree::isLeaf() const {
	return children.empty();
}

void VaporOctree::drawLeafs(float minDensity, float maxDensity) const {
	if(this->isLeaf()){
		if(this->particlesIndex.empty()){
			ofSetColor(255, 20);
		}else{
			auto alpha = ofMap(density, minDensity, maxDensity, 0, 255);
			ofSetColor(255, alpha);
		}
		ofDrawBox(bb.center, bb.size.x, bb.size.y, bb.size.z);
	}else{
		for(const auto & child : children){
			child.drawLeafs(minDensity, maxDensity);
		}
	}
}

size_t VaporOctree::getMaxLevel() const {
	size_t level = 0;
	for(const auto & child : children){
		level = std::max(level, child.getMaxLevel(1));
	}
	return level;
}

size_t VaporOctree::getMaxLevel(size_t current) const {
	auto max = current;
	for(const auto & child : children){
		max = std::max(max, child.getMaxLevel(current + 1));
	}
	return max;
}

size_t VaporOctree::size() const {
	if(isLeaf()){
		return particlesIndex.size();
	}else{
		return std::accumulate(children.begin(), children.end(), size_t(0), [] (size_t acc, const VaporOctree &octree)->size_t {
			                       return acc + octree.size();
		                       });
	}
}

std::vector <Particle> VaporOctree::toVector() const {
	if(isLeaf() && !particlesIndex.empty()){
		return {Particle {bb.center, std::max(bb.size.x, std::max(bb.size.y, bb.size.z)), density}};
	}else{
		return std::accumulate(children.begin(), children.end(), std::vector <Particle>(), [&](std::vector <Particle> &vec, const VaporOctree &octree){
			auto child = octree.toVector();
			vec.insert(vec.end(), child.begin(), child.end());
			return vec;
		});
	}
}

ofFloatPixels VaporOctree::getPixels(size_t z, float minDensity, float maxDensity) const {
	ofFloatPixels pixels;
	size_t size = pow(2, resolution);
	pixels.allocate(size, size, 1);
	for(size_t y = 0, i = 0; y < size; ++y){
		for(size_t x = 0; x < size; ++x, i++){
			pixels[i] = getDensity(x, y, z) > 0 ? 1.0 : 0.0; //ofMap(getDensity(x,y,z), minDensity, maxDensity, 0, 1.0);
		}
	}
	return std::move(pixels);
}


float VaporOctree::getDensity(size_t x, size_t y, size_t z) const {
	if(children.empty()){
		return density;
	}else{
		if(x > this->x){
			if(y > this->y){
				if(z > this->z){
					return children[7].getDensity(x, y, z);
				}else{
					return children[6].getDensity(x, y, z);
				}
			}else{
				if(z > this->z){
					return children[5].getDensity(x, y, z);
				}else{
					return children[4].getDensity(x, y, z);
				}
			}
		}else{
			if(y > this->y){
				if(z > this->z){
					return children[3].getDensity(x, y, z);
				}else{
					return children[2].getDensity(x, y, z);
				}
			}else{
				if(z > this->z){
					return children[1].getDensity(x, y, z);
				}else{
					return children[0].getDensity(x, y, z);
				}
			}
		}
	}
}
