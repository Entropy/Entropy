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
        
        static const unsigned POINTS_START_SIZE = 20;
        
        enum Location
        {
            X_SIDE = 0x01,
            Y_SIDE = 0x02,
            Z_SIDE = 0x04
        };
        
        static void setMaxDepth(unsigned maxDepth) { Octree<T>::maxDepth = maxDepth; }
        
        Octree();
        
        void init(const ofVec3f& min, const ofVec3f& max, unsigned depth = 0);
        
        void addPoint(T& point);
        
        void addPoints(vector<T>& points) { addPointsParallel(points); }
        void addPoints(T* points, unsigned numPoints) { addPointsParallel(points, numPoints); }
        
        void addPointsSerial(vector<T>& points);
        void addPointsSerial(T* points, unsigned numPoints);
        
        void addPointsParallel(vector<T>& points);
        void addPointsParallel(T* points, unsigned numPoints);
        
        void clear();
        
        void debugDraw();
        
        //void buildEmpty(unsigned depth, unsigned numPoints);
    
    private:
        static unsigned maxDepth;
        static ofVboMesh boxMesh;
        
        // hopefully after the first few iterations this shouldn't be resized too often
        tbb::concurrent_vector<T*> points;
        // save the number of points so that we don't have to keep reallocating the vector
        tbb::atomic<unsigned> numPoints;
        ofVec3f min, max, mid;
        Octree* children;
        unsigned depth;
        bool hasPoints;
        float mass;
        ofVec3f massCenterOfMassProduct;
    };
}

#include "Octree.inl"
