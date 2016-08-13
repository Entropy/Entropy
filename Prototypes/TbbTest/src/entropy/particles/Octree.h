/*
 *  Octree.h
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

/* Currently this octree just builds to a maximum depth rather than a number of particles
 * per subdivision that way memory allocation has to only happen once and particles don't have
 * to be moved around if a subdivision becomes too full */

#pragma once

#include "ofMain.h"

#include "tbb/tbb.h"

namespace nm
{
	template<class T>
	class Octree
	{
	public:
		typedef shared_ptr<Octree> Ptr;

		// hacky C++11 way to get around static const vars in template class
		// have actually made a cpp class now so can change these back to 
		// vars rather than functions
		static constexpr unsigned POINTS_START_SIZE() { return 40; }
		static constexpr unsigned MAX_DEPTH() { return 4; }
		static constexpr float THETA() { return .5f; }
		//static constexpr float FORCE_MULTIPLIER() { return 5e7; }
		static constexpr float INTERACTION_DISTANCE() { return 2.f; }

		static void setForceMultiplier(float forceMultiplier) { Octree::forceMultiplier = forceMultiplier; }

		enum Location
		{
			X_SIDE = 0x01,
			Y_SIDE = 0x02,
			Z_SIDE = 0x04
		};

		Octree();
		~Octree();

		void init(const ofVec3f& min, const ofVec3f& max, unsigned depth = 0);

		//void updateCenterOfMass();

		void updateCenterOfCharge();

		// if close enough to another point to annihilate it, return that point
		T* sumForces(T& point);// , float forceMultiplier);

		void addPoint(T& point);

		void addPoints(vector<T>& points) { addPointsParallel(points); }
		void addPoints(T* points, unsigned numPoints) { addPointsParallel(points, numPoints); }

		void addPointsSerial(vector<T>& points);
		void addPointsSerial(T* points, unsigned numPoints);

		void addPointsParallel(vector<T>& points);
		void addPointsParallel(T* points, unsigned numPoints);

		void clear();

		void debugDraw(unsigned depth);

		void addChildren(bool recursive);

		inline float getCharge() const { return charge; }

		inline float getAbsCharge() const { return absCharge; }

		inline ofVec3f getCenterOfCharge() const { return centerOfCharge; }

		Octree* getChildren() const { return children; }

		inline ofVec3f getMax() const { return max; }
		inline ofVec3f getMin() const { return min; }

	private:
		static ofVboMesh boxMesh;
		static unsigned numOctrees;
		static float forceMultiplier;

		// hopefully after the first few iterations this shouldn't be resized too often
		tbb::concurrent_vector<T*> points;
		// save the number of points so that we don't have to keep reallocating the vector
		tbb::atomic<unsigned> numPoints;
		ofVec3f min, max, mid;
		float size;
		Octree* children;
		unsigned depth;
		bool hasPoints;
		float charge; // mass for gravity, charge for electromagnetic systems
		float absCharge;
		ofVec3f centerOfCharge;
	};
}

#include "Octree.inl"
