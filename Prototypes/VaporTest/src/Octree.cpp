#include "Octree.h"
#include <algorithm>
#include "of3dGraphics.h"
#include "ofGraphics.h"
#include "ofUtils.h"
#include "ThreadPool.h"
#include <future>

struct BoundingBoxSearch{
	inline BoundingBoxSearch(const ofVec3f & min, const ofVec3f & max, float density)
	    :min(min)
	    ,max(max)
	    ,density(density){}
	BoundingBoxSearch(){}
	ofVec3f min = {std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};
	ofVec3f max = {std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest()};
	float density = 0.0f;
};

Octree::Octree()
:density(0.f)
{

}

void Octree::setup(const std::vector<Particle> & particles){
	this->particles = particles;
	auto bb = std::accumulate(particles.begin(), particles.end(), BoundingBoxSearch(), [](const BoundingBoxSearch & box, const Particle & p) -> BoundingBoxSearch{
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
	this->density = bb.density;// / float(particles.size());
	//cout << "octree setup bb: " << bb.min << " <-> " << bb.max << endl;
}


bool Octree::divide(size_t resolution, float minDensity, float maxDensity){
	float span = maxDensity - minDensity;
	float thresDensity = minDensity + span*0.0005;
	if(density<thresDensity){
		particles.clear();
		density=0;
	}
	if(resolution==0 || particles.size() == 0){
		return false;
	}
	auto bbmin = this->bb.min;
	auto bbmax = this->bb.max;
	ofVec3f p1 = bbmin;
	ofVec3f p2 = {bbmin.x, bbmin.y, bbmax.z};
	ofVec3f p3 = {bbmin.x, bbmax.y, bbmin.z};
	ofVec3f p4 = {bbmin.x, bbmax.y, bbmax.z};
	ofVec3f p5 = {bbmax.x, bbmin.y, bbmin.z};
	ofVec3f p6 = {bbmax.x, bbmin.y, bbmax.z};
	ofVec3f p7 = {bbmax.x, bbmax.y, bbmin.z};
	ofVec3f p8 = bbmax;

	ofVec3f center = (p1 + p8) * 0.5;

	children.resize(8);
	children[0].bb = BoundingBox::fromTwoPoints(p1, center);
	children[1].bb = BoundingBox::fromTwoPoints(p2, center);
	children[2].bb = BoundingBox::fromTwoPoints(p3, center);
	children[3].bb = BoundingBox::fromTwoPoints(p4, center);
	children[4].bb = BoundingBox::fromTwoPoints(p5, center);
	children[5].bb = BoundingBox::fromTwoPoints(p6, center);
	children[6].bb = BoundingBox::fromTwoPoints(p7, center);
	children[7].bb = BoundingBox::fromTwoPoints(p8, center);

	std::array<float,8> distances{{0.f}};

	for(auto & p: particles){
		distances[0] = children[0].bb.center.squareDistance(p.pos);
		distances[1] = children[1].bb.center.squareDistance(p.pos);
		distances[2] = children[2].bb.center.squareDistance(p.pos);
		distances[3] = children[3].bb.center.squareDistance(p.pos);
		distances[4] = children[4].bb.center.squareDistance(p.pos);
		distances[5] = children[5].bb.center.squareDistance(p.pos);
		distances[6] = children[6].bb.center.squareDistance(p.pos);
		distances[7] = children[7].bb.center.squareDistance(p.pos);
		auto min = std::min_element(distances.begin(), distances.end()) - distances.begin();
		children[min].particles.push_back(p);
		children[min].density += p.density;
	}

	//this->particles.clear();
	return true;
}

void Octree::compute(size_t resolution, float minDensity, float maxDensity){
	if(this->divide(resolution, minDensity, maxDensity)){
		std::array<std::future<bool>, 8> futures;
		for(size_t i = 0; i<children.size(); ++i){
			auto & child = children[i];
			/*ThreadPool::pool().addTask([&child, &promise, i, resolution, minDensity, maxDensity]{
				child.compute(resolution - 1, minDensity, maxDensity, 0);
				promise.set_value(true);
			});*/
			futures[i] = std::async(std::launch::async, [&child, resolution, minDensity, maxDensity]{
				child.compute(resolution - 1, minDensity, maxDensity, 0);
				return true;
			});
		}

		for(auto & future: futures){
			future.wait();
		}
	}
}

void Octree::compute(size_t resolution, float minDensity, float maxDensity, size_t level){
	if(this->divide(resolution, minDensity, maxDensity)){
		for(auto & child: children){
			child.compute(resolution - 1, minDensity, maxDensity, level+1);
		}
	}
}


bool Octree::isLeaf() const{
	return children.empty();
}

void Octree::drawLeafs(float minDensity, float maxDensity) const{
	if(this->isLeaf()){
		if(this->particles.empty()){
			return; //ofSetColor(255,20);
		}else{
			auto alpha = ofMap(density, minDensity, maxDensity, 0, 255);
			//cout << gray << endl;
			ofSetColor(255, alpha);
		}
		ofDrawBox(bb.center, bb.size.x, bb.size.y, bb.size.z);
	}else{
		for(const auto & child: children){
			child.drawLeafs(minDensity, maxDensity);
		}
	}
}

size_t Octree::getMaxLevel() const{
	size_t level = 0;
	for(const auto & child: children){
		level = std::max(level, child.getMaxLevel(1));
	}
	return level;
}

size_t Octree::getMaxLevel(size_t current) const{
	auto max = current;
	for(const auto & child: children){
		max = std::max(max, child.getMaxLevel(current+1));
	}
	return max;
}

size_t Octree::size() const{
	if(isLeaf()){
		return particles.size();
	}else{
		return std::accumulate(children.begin(), children.end(), size_t(0), [](size_t acc, const Octree & octree) -> size_t{
			return acc + octree.size();
		});
	}
}

std::vector<Particle> Octree::toVector() const{
	if(isLeaf()){
		return {Particle{bb.center, std::max(bb.size.x, std::max(bb.size.y, bb.size.z)), density}};
	}else{
		return std::accumulate(children.begin(), children.end(), std::vector<Particle>(), [&](std::vector<Particle> & vec, const Octree & octree){
			auto child = octree.toVector();
			vec.insert(vec.end(), child.begin(), child.end());
			return vec;
		});
	}
}
