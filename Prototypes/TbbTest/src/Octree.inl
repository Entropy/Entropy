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
    ofVboMesh Octree<T>::boxMesh;
    
    template<class T>
    Octree<T>::Octree() :
        children(NULL),
        points(POINTS_START_SIZE), // means that this vector won't be fragmented for the first pointsStartSize elements
        numPoints(0),
        hasPoints(false),
        mass(0.f)
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
    void Octree<T>::clear()
    {
        hasPoints = false;
        numPoints = 0;
        mass = 0.f;
        massCenterMassProduct.set(0.f);
        if (children)
        {
            for (unsigned i = 0; i < 8; ++i) children[i].clear();
        }
    }
    
    template<class T>
    void Octree<T>::addPointsSerial(vector<T>& points)
    {
        addPointsSerial(points.getPtr(), points.size());
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
        addPointsParallel(points.getPtr(), points.size());
    }
    
    template<class T>
    void Octree<T>::addPointsParallel(T* points, unsigned numPoints)
    {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, numPoints),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for(size_t i = r.begin(); i != r.end(); ++i) addPoint(points[i]);
                          });
    }
    
    template<class T>
    void Octree<T>::debugDraw()
    {
        if (boxMesh.getNumVertices() == 0) boxMesh = ofMesh::box(1.f, 1.f, 1.f, 1, 1, 1);
        if (hasPoints)
        {
            ofPushMatrix();
            ofTranslate(mid);
            ofScale(max.x - min.x, max.y - min.y, max.z - min.z);
            boxMesh.drawWireframe();
            ofPopMatrix();
            if (children)
            {
                for (unsigned i = 0; i < 8; ++i) children[i].debugDraw();
            }
        }
    }
    
    template<class T>
    void Octree<T>::addPoint(T& point)
    {
        hasPoints = true;
        mass += point.getMass();
        massCenterMassProduct += point.getMass() * point;
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
}
