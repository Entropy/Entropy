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
		numPoints(0),
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
		if (depth == MAX_DEPTH()) points.resize(POINTS_START_SIZE());
	}

	template<class T>
	void Octree<T>::updateCenterOfCharge()
	{
		if (hasPoints)
		{
			if (depth == MAX_DEPTH())
			{
				for (unsigned i = 0; i < numPoints; ++i)
				{
					charge += points[i]->getCharge();
					absCharge += abs(points[i]->getCharge());
					centerOfCharge += *points[i] * abs(points[i]->getCharge());
				}
			}
			else if (children)
			{
				tbb::task_group taskGroup;
				for (unsigned i = 0; i < 8; ++i)
				{
					taskGroup.run([i, this] { children[i].updateCenterOfCharge(); });
				}
				taskGroup.wait();

				/*
				tbb::parallel_for(tbb::blocked_range<size_t>(0, 8),
				[&](const tbb::blocked_range<size_t>& r) {
				for(size_t i = r.begin(); i != r.end(); ++i) children[i].updateCenterOfCharge();
				});
				*/

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
	T* Octree<T>::sumForces(T& point)
	{
		if (hasPoints)
		{
			T* annihilate = NULL;
			if (depth < MAX_DEPTH())
			{
				ofVec3f direction = centerOfCharge - point;
				float distSq = direction.lengthSquared();
				float dist = sqrt(distSq);
				if (dist > ANNIHILATION_DISTANCE() && size / dist < THETA())
				{
					// far enough away to use this node
					point.setForce(point.getForce() - FORCE_MULTIPLIER() * direction * point.getCharge() * charge / (distSq * dist));
				}
				else if (children)
				{
					for (unsigned i = 0; i < 8; ++i)
					{
						if (annihilate) children[i].sumForces(point);
						else annihilate = children[i].sumForces(point);
					}
				}
			}
			else
			{
				for (unsigned i = 0; i < numPoints; ++i)
				{
					if (&point != points[i])
					{
						ofVec3f direction = centerOfCharge - point;
						float distSq = direction.lengthSquared();
						float dist = sqrt(distSq);
						point.setForce(point.getForce() - FORCE_MULTIPLIER() * direction * point.getCharge() * charge / (distSq * dist));
						if (dist < ANNIHILATION_DISTANCE() &&
							((point.getType() == Particle::ANTI_UP_QUARK && points[i]->getType() == Particle::UP_QUARK) ||
							(point.getType() == Particle::UP_QUARK && points[i]->getType() == Particle::ANTI_UP_QUARK) ||
								(point.getType() == Particle::ELECTRON && points[i]->getType() == Particle::POSITRON) ||
								(point.getType() == Particle::POSITRON && points[i]->getType() == Particle::ELECTRON))) annihilate = points[i];
					}
				}
			}
			return annihilate;
		}
		return NULL;
	}

	template<class T>
	void Octree<T>::clear()
	{
		hasPoints = false;
		numPoints = 0;
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
	}

	template<class T>
	void Octree<T>::addPointsSerial(vector<T>& points)
	{
		addPointsSerial(points.getPtr(), numPoints);
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
		addPointsParallel(points.getPtr(), numPoints);
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
		//mass += point.getMass();
		//charge += point.getCharge();
		//centerOfMass += point.getMass() * point;
		//centerOfCharge += point.getCharge() * point;
		if (depth == MAX_DEPTH())
		{
			//points.push_back(&point);
			unsigned idx = numPoints.fetch_and_increment();
			points.grow_to_at_least(idx + 1);
			points[idx] = &point;
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
			if (point.x > mid.x) octant |= X_SIDE;
			if (point.y > mid.y) octant |= Y_SIDE;
			if (point.z > mid.z) octant |= Z_SIDE;
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

