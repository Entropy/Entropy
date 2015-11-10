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

#pragma once

#include "cinder/AxisAlignedBox.h"

class Grid {
public:
    typedef std::vector<uint32_t> Cell;
    
    Grid(){}
    Grid( const ci::AxisAlignedBox3f &bounds, uint32_t k );
    
    
    Cell* addPoint( const ci::vec3 &position, uint32_t indice );
    
    void resize( const ci::AxisAlignedBox3f &bounds, uint32_t k );
    void resize( uint32_t k );
    
    ci::AxisAlignedBox3f    getBounds() const { return mBounds; }
    ci::vec3                getNumCells() const { return mNumCells; }
    uint32_t                getCellSize() const { return mCellSize; }
    uint32_t                getK() const { return mK; }
    
    Cell                    getRegion( const ci::ivec3 &position, float radius );
    Cell                    getRegion( const ci::ivec3 &min, const ci::ivec3 &max );
    std::vector<Cell*>      getRegionCells( const ci::ivec3 &position, float radius );
    std::vector<Cell*>      getRegionCells( const ci::ivec3 &min, const ci::ivec3 &max );
    
    //void update( Solver* solver );
    
private:
    std::vector< Cell >    mGrid;
    ci::ivec3              mNumCells, mOffset;
    ci::AxisAlignedBox3f   mBounds;
    uint32_t               mK, mCellSize;
};