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

#include "tbb/concurrent_vector.h"

namespace nm
{
    template<class T>
    class Octree
    {
    public:
        typedef shared_ptr<Octree> Ptr;
        
        enum Location
        {
            X_SIDE = 0x01,
            Y_SIDE = 0x02,
            Z_SIZE = 0x04
        };
        
        static void setMaxDepth(unsigned maxDepth) { Octree::maxDepth = maxDepth; }
        
        Octree(unsigned depth = 0);
        
        void addPoint(shared_ptr<T> point);
        
        //void buildEmpty(unsigned depth, unsigned numPoints);
    
    private:
        static unsigned maxDepth;
        
        // hopefully after the first few iterations this shouldn't be resized too often
        tbb::concurrent_vector<shared_ptr<T> > points;
        // save the number of points so that we don't have to keep reallocating the vector
        unsigned numPoints;
        ofVec3f min, max;
        Octree* children;
        unsigned depth;
    };
}

#include "Octree.inl"
