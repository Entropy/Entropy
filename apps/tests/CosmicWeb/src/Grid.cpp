/*
 Entropy Toolkit
 Copyright (c) 2015, Simon Geilfus - All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
 the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#include "Grid.h"

using namespace ci;
using namespace std;

Grid::Grid( const AxisAlignedBox3f &bounds, uint32_t k )
{
    resize( bounds, k );
}


/*void Grid::update( Solver* solver )
{
    int n = mGrid.size();
    for(int i = 0; i < n; i++) {
        mGrid[i].clear();
    }
    
    n = solver->size();
    int x, y, z, j;
    for(int i = 0; i < n; i++) {
        const ParticleRef& p = solver->getParticle( i );
        x = ( ( unsigned )  p->x + mOffset.x ) >> mK;
        y = ( ( unsigned ) p->y + mOffset.y ) >> mK;
        z = ( ( unsigned ) p->z + mOffset.z ) >> mK;
        
        j = x + mNumCells.x*y + mNumCells.x*mNumCells.y*z ;
        
        if( j < mGrid.size() ){
            mGrid[ j ].push_back( i );
            mGrid[ j ].mAvgPosition += p->xyz();
            mGrid[ j ].mAvgVelocity += ( p->xyz() - p->getPreviousState() );
        }
    }
    n = mGrid.size();
    for(int i = 0; i < n; i++) {
        if( mGrid[i].size() ){
            mGrid[i].mAvgPosition /= (float) mGrid[i].size();
            mGrid[i].mAvgVelocity /= (float) mGrid[i].size();
        }
    }
    
}*/


Grid::Cell* Grid::addPoint( const ci::vec3 &position, uint32_t indice )
{
    int x = ( ( uint32_t ) position.x + mOffset.x ) >> mK;
    int y = ( ( uint32_t ) position.y + mOffset.y ) >> mK;
    int z = ( ( uint32_t ) position.z + mOffset.z ) >> mK;
    int j = x + mNumCells.x*y + mNumCells.x*mNumCells.y*z ;
    
    if( j < mGrid.size() ){
        mGrid[j].push_back( indice );
        return &mGrid[j];
    }
    else return nullptr;
}

void Grid::resize( const AxisAlignedBox3f &bounds, uint32_t k )
{
    mBounds = bounds;
    resize( k );
}
void Grid::resize( uint32_t k )
{
    mK          = k;
    mCellSize   = 1 << k;
    mOffset     = glm::abs( mBounds.getMin() );
    mNumCells   = glm::ceil( mBounds.getSize() / (float) mCellSize );
    mGrid.clear();
    mGrid.resize( mNumCells.x * mNumCells.y * mNumCells.z );
}

Grid::Cell Grid::getRegion( const ivec3 &position, float radius )
{
    ivec3 radiusVec = ivec3( radius, radius, radius );
    ivec3 min       = glm::max( glm::min( position - radiusVec, ivec3( mBounds.getMax() ) - ivec3( 1 ) ), ivec3( mBounds.getMin() ) );
    ivec3 max       = glm::max( glm::min( position + radiusVec, ivec3( mBounds.getMax() ) - ivec3( 1 ) ), ivec3( mBounds.getMin() ) );

    return getRegion( min, max );
}

Grid::Cell Grid::getRegion( const ivec3 &min, const ivec3 &max )
{
    Grid::Cell region;
    ivec3 minCell = ivec3( ( min.x + mOffset.x ) >> mK, ( min.y + mOffset.y ) >> mK, ( min.z + mOffset.z ) >> mK );
    ivec3 maxCell = glm::min( 1 + ivec3( ( max.x + mOffset.x ) >> mK, ( max.y + mOffset.y ) >> mK, ( max.z + mOffset.z ) >> mK ), mNumCells );
    for(unsigned z = minCell.z; z < maxCell.z; z++) {
        for(unsigned y = minCell.y; y < maxCell.y; y++) {
            for(unsigned x = minCell.x; x < maxCell.x; x++) {
                const Cell& cur = mGrid[ x + mNumCells.x*y + mNumCells.x*mNumCells.y*z ];
                region.insert( region.end(), cur.begin(), cur.end() );
            }
        }
    }
    return region;
}


std::vector<Grid::Cell*> Grid::getRegionCells( const ivec3 &position, float radius )
{
    ivec3 radiusVec = ivec3( radius, radius, radius );
    ivec3 min       = glm::max( glm::min( position - radiusVec, ivec3( mBounds.getMax() ) - ivec3( 1 ) ), ivec3( mBounds.getMin() ) );
    ivec3 max       = glm::max( glm::min( position + radiusVec, ivec3( mBounds.getMax() ) - ivec3( 1 ) ), ivec3( mBounds.getMin() ) );
    
    return getRegionCells( min, max );
}
std::vector<Grid::Cell*> Grid::getRegionCells( const ivec3 &min, const ivec3 &max )
{
    std::vector<Grid::Cell*> regionCells;
    ivec3 minCell( ( min.x + mOffset.x ) >> mK, ( min.y + mOffset.y ) >> mK, ( min.z + mOffset.z ) >> mK );
    ivec3 maxCell = glm::min( 1 + ivec3( ( max.x + mOffset.x ) >> mK, ( max.y + mOffset.y ) >> mK, ( max.z + mOffset.z ) >> mK ), mNumCells );    
    for(unsigned z = minCell.z; z < maxCell.z; z++) {
        for(unsigned y = minCell.y; y < maxCell.y; y++) {
            for(unsigned x = minCell.x; x < maxCell.x; x++) {
                regionCells.push_back( &mGrid[ x + mNumCells.x*y + mNumCells.x*mNumCells.y*z ] );
            }
        }
    }
    return regionCells;
}