/*
 *  Octree.cpp
 *
 *  Copyright (c) 2016, Neil Mendoza, http://www.neilmendoza.com
 *  All rights reserved. 
 *  
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions are met: 
 *  
 *  * Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 *  * Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *  * Neither the name of Neil Mendoza nor the names of its contributors may be used 
 *    to endorse or promote products derived from this software without 
 *    specific prior written permission. 
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE. 
 *
 */

#include "Octree.h"

namespace nm
{        
	template<class T>
	Octree<T>::Octree() :
		children(NULL),
		hasPoints(false),
		charge(0.f),
		absCharge(0.f),
		depth(0)
		//mass(0.f)
	{
	}

	template<class T>
	Octree<T>::~Octree()
	{
		delete[] children;
	}

	template<class T>
	void Octree<T>::init(const ofVec3f& min, const ofVec3f& max, unsigned depth)
	{
		this->min = min;
		this->max = max;
		this->size = ((max.x - min.x) + (max.y - min.y) + (max.z - min.z)) / 3.f;
		this->mid = .5f * (min + max);
		this->depth = depth;
		if (depth == MAX_DEPTH()) points.reserve(POINTS_START_SIZE());
	}

	template<class T>
	void Octree<T>::updateCenterOfCharge()
	{
		if (hasPoints)
		{
			if (depth == MAX_DEPTH())
			{
				for (auto * point: points)
				{
					charge += point->getCharge();
					absCharge += abs(point->getCharge());
					centerOfCharge += point->pos * abs(point->getCharge());
				}
			}
			else if (children)
			{
				tbb::task_group taskGroup;
				for (unsigned i = 0; i < 8; ++i)
				{
					taskGroup.run([i, this] {
						children[i].updateCenterOfCharge();
					});
				}
				taskGroup.wait();

				for (unsigned i = 0; i < 8; ++i)
				{
					absCharge += children[i].getAbsCharge();
					charge += children[i].getCharge();
					centerOfCharge += children[i].getAbsCharge() * children[i].getCenterOfCharge();
				}
			}
			centerOfCharge = centerOfCharge / absCharge;
		}

		/*
		centerOfMass = centerOfMass / mass;
		if (children)
		{
		tbb::parallel_for(tbb::blocked_range<size_t>(0, 8),
		[&](const tbb::blocked_range<size_t>& r) {
		for(size_t i = r.begin(); i != r.end(); ++i) children[i].updateCenterOfForce();
		});
		//tbb::task_group taskGroup;
		//for (unsigned i = 0; i < 8; ++i)
		//{
		//    const unsigned idx = i;
		//    taskGroup.run([&]{ children[idx].updateCenterOfMass(); });
		//}
		//taskGroup.wait();
		}*/
	}

	template<class T>
	void Octree<T>::sumForces(T& point)//, float forceMultiplier)
	{
		if (hasPoints)
		{
			if (depth < MAX_DEPTH())
			{
				ofVec3f direction = centerOfCharge - point.pos;
				float distSq = direction.lengthSquared();
				float dist = sqrt(distSq);
				if (dist > INTERACTION_DISTANCE() && size / dist < THETA())
				{
					// far enough away to use this node
					point.setForce(point.getForce() - forceMultiplier * direction * point.getCharge() * charge / (distSq * dist));
				}
				else if (children)
				{
					for (unsigned i = 0; i < 8; ++i)
					{
						children[i].sumForces(point);// , forceMultiplier);
					}
				}
			}
			else
			{
				auto i = 0;
				for (auto * other: points)
				{
					i++;
					if (&point != other)
					{
						ofVec3f direction = centerOfCharge - point.pos;
						float distSq = direction.lengthSquared();
						float dist = sqrt(distSq);
						point.setForce(point.getForce() - forceMultiplier * direction * point.getCharge() * charge / (distSq * dist));
						// CHECK THAT ^ IS ACTUALLY XOR
						if (dist < CANDIDATE_DISTANCE() &&
							(((point.getAnnihilationFlag() ^ other->getAnnihilationFlag()) == 0xFF) ||
							((point.getFusion1Flag() ^ other->getFusion1Flag()) == 0xFF) ||
							((point.getFusion2Flag() ^ other->getFusion2Flag()) == 0xFF))){
							point.potentialInteractionPartners.push_back(other);
						}
					}
				}
			}
		}
	}

	template<class T>
	void Octree<T>::findNearestThan(const T& point, float distance, std::vector<T*> & nearList) const{
		if(hasPoints){
			if (depth < MAX_DEPTH())
			{
				if (children)
				{
					unsigned char octant = 0x00;
					if (point.pos.x > mid.x) octant |= X_SIDE;
					if (point.pos.y > mid.y) octant |= Y_SIDE;
					if (point.pos.z > mid.z) octant |= Z_SIDE;
					children[octant].findNearestThan(point, distance, nearList);
				}
			}
			else
			{
				for (auto * other: points)
				{
					if (&point > other)
					{
						ofVec3f direction = centerOfCharge - point.pos;
						float distSq = direction.lengthSquared();
						float dist = sqrt(distSq);
						if (dist < distance){
							nearList.push_back(other);
						}
					}
				}
			}
		}
	}


	template<class T>
	template<typename Type>
	void Octree<T>::findNearestThanByType(const T& point, float distance, std::initializer_list<Type> allowedTypes, std::vector<T*> & nearList) const{
		if(hasPoints){
			if (depth < MAX_DEPTH())
			{
				if (children)
				{
					unsigned char octant = 0x00;
					if (point.pos.x > mid.x) octant |= X_SIDE;
					if (point.pos.y > mid.y) octant |= Y_SIDE;
					if (point.pos.z > mid.z) octant |= Z_SIDE;
					children[octant].findNearestThanByType(point, distance, allowedTypes, nearList);
				}
			}
			else
			{
				for (auto * other: points)
				{
					if (&point > other)
					{
						bool allowedType = false;
						for(auto t: allowedTypes){
							allowedType |= (t == other->getType());
						}
						if(!allowedType){
							continue;
						}
						ofVec3f direction = centerOfCharge - point.pos;
						float distSq = direction.lengthSquared();
						float dist = sqrt(distSq);
						if (dist < distance){
							nearList.push_back(other);
						}
					}
				}
			}
		}
	}

	template<class T>
	void Octree<T>::clear()
	{
		hasPoints = false;
		//points.clear();
		//mass = 0.f;
		charge = 0.f;
		absCharge = 0.f;
		//centerOfMass.set(0.f);
		centerOfCharge.set(0.f);
		if (children)
		{
			tbb::parallel_for(tbb::blocked_range<size_t>(0, 8),
				[&](const tbb::blocked_range<size_t>& r) {
				for (size_t i = r.begin(); i != r.end(); ++i) children[i].clear();
			});
			/*
			tbb::task_group taskGroup;
			for (unsigned i = 0; i < 8; ++i)
			{
			const unsigned idx = i;
			taskGroup.run([&]{ children[idx].clear(); });
			}
			taskGroup.wait();*/
		}
		if(!points.empty()){
			points.clear();
		}
	}


	template<class T>
	template<size_t N>
	void Octree<T>::addPointsSerial(std::array<T,N>& points)
	{
		addPointsSerial(points.data(), points.size());
	}

	template<class T>
	void Octree<T>::addPointsSerial(vector<T>& points)
	{
		addPointsSerial(points.data(), points.size());
	}

	template<class T>
	void Octree<T>::addPointsSerial(T* points, unsigned numPoints)
	{
		for (unsigned i = 0; i < numPoints; ++i)
		{
			addPoint(points[i]);
		}
	}

	template<class T>
	void Octree<T>::addPointsParallel(vector<T>& points)
	{
		addPointsParallel(points.data(), points.size());
	}

	template<class T>
	template<size_t N>
	void Octree<T>::addPointsParallel(std::array<T,N>& points)
	{
		addPointsParallel(points.data(), points.size());
	}

	template<class T>
	void Octree<T>::addPointsParallel(T* points, unsigned numPoints)
	{
		tbb::parallel_for(tbb::blocked_range<size_t>(0, numPoints),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i != r.end(); ++i) addPoint(points[i]);
		});
	}

	template<class T>
	void Octree<T>::addChildren(bool recursive)
	{
		if (children == NULL)
		{
			children = new Octree[8]();
			for (unsigned i = 0; i < 8; ++i)
			{
				ofVec3f childMin(mid);
				ofVec3f childMax(mid);

				if (i & X_SIDE) childMax.x = max.x;
				else childMin.x = min.x;

				if (i & Y_SIDE) childMax.y = max.y;
				else childMin.y = min.y;

				if (i & Z_SIDE) childMax.z = max.z;
				else childMin.z = min.z;

				children[i].init(childMin, childMax, depth + 1);

				if (recursive && depth < MAX_DEPTH() - 1)
				{
					children[i].addChildren(recursive);
				}
			}
		}
	}

	template<class T>
	void Octree<T>::addPoint(T& point)
	{
		hasPoints = true;
		if (depth == MAX_DEPTH())
		{
			points.push_back(&point);
		}
		else
		{
			if (children == NULL)
			{
				children = new Octree[8]();
				for (unsigned i = 0; i < 8; ++i)
				{
					ofVec3f childMin(mid);
					ofVec3f childMax(mid);

					if (i & X_SIDE) childMax.x = max.x;
					else childMin.x = min.x;

					if (i & Y_SIDE) childMax.y = max.y;
					else childMin.y = min.y;

					if (i & Z_SIDE) childMax.z = max.z;
					else childMin.z = min.z;

					children[i].init(childMin, childMax, depth + 1);
				}
			}
			unsigned char octant = 0x00;
			if (point.pos.x > mid.x) octant |= X_SIDE;
			if (point.pos.y > mid.y) octant |= Y_SIDE;
			if (point.pos.z > mid.z) octant |= Z_SIDE;
			children[octant].addPoint(point);
		}
	}

	template<class T>
	void Octree<T>::debugDraw(unsigned depth)
	{
		if (boxMesh.getNumVertices() == 0) boxMesh = ofMesh::box(1.f, 1.f, 1.f, 1, 1, 1);
		if (hasPoints)
		{
			if (this->depth == depth)
			{
				ofPushMatrix();
				ofTranslate(mid);
				ofScale(max.x - min.x, max.y - min.y, max.z - min.z);
				boxMesh.drawWireframe();
				ofPopMatrix();
			}
			else if (children)
			{
				for (unsigned i = 0; i < 8; ++i) children[i].debugDraw();
			}
		}
	}
}

