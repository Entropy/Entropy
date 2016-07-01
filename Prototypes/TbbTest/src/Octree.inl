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

namespace nm
{
    template<class T>
    unsigned Octree<T>::maxDepth = 8;
    
    template<class T>
    Octree<T>::Octree() :
        children(NULL),
        points(POINTS_START_SIZE), // means that this vector won't be fragmented for the first pointsStartSize elements
        numPoints(0)
    {
    }
    
    template<class T>
    void Octree<T>::init(const ofVec3f& min, const ofVec3f& max, unsigned depth)
    {
        this->min = min;
        this->max = max;
        this->mid = .5f * (min + max);
        this->depth = depth;
    }
    
    template<class T>
    void Octree<T>::addPoint(T& point)
    {
        if (depth == Octree::maxDepth)
        {
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
                    
                    if (i & X_SIDE) childMax.x = childMax.x;
                    else childMin.x = min.x;
                    
                    if (i & Y_SIDE) childMax.y = childMax.y;
                    else childMin.y = min.y;
                    
                    if (i & Z_SIDE) childMax.z = childMax.z;
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
}
