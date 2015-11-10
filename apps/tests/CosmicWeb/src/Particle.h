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
#include "cinder/Vector.h"
#include "cinder/gl/VboMesh.h"

#include <cereal/types/memory.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>

#include <deque>


//typedef std::shared_ptr<class Particle>     ParticleRef;
//typedef std::shared_ptr<class ParticleSet>  ParticleSetRef;

class Particle {
public:
    //static ParticleRef create( const ci::vec3 &initial, const ci::vec3 &final, uint32_t particleId );
    
    Particle(){}
    Particle( const ci::vec3 &initial, const ci::vec3 &final, uint32_t indice );
    
    const ci::vec3& getPosition() const { return mInitial; }
    const ci::vec3& getVelocity() const { return mFinal; }
    uint32_t        getIndice() const { return mIndice; }
    
    float	        getDensity() const { return mDensity; }
    void	        setDensity( float density ) { mDensity = density; }
    
    template <class Archive>
    void serialize( Archive & ar );
protected:
    uint32_t    mIndice;
    float       mDensity;
    ci::vec3    mInitial;
    ci::vec3    mFinal;
};

class ParticleSet {
public:
    //static ParticleSetRef create();
    
    ParticleSet();
    
    void addParticle( const ci::vec3 &initial, const ci::vec3 &final, uint32_t particleId );
    
    const ci::gl::VboMeshRef&   getVboMesh();
    const ci::AxisAlignedBox3f& getBounds() const { return mBounds; }
    
    const std::deque<Particle>& getParticles() const { return mParticles; }
    uint32_t                    getNumParticles() const { return mParticles.size(); }
    
    void write( const ci::fs::path &filepath );
    void read( const ci::fs::path &filepath, const ci::fs::path &density = ci::fs::path() );
    void merge( const std::vector<ci::fs::path> &filepaths, const std::vector<ci::fs::path> &densities = std::vector<ci::fs::path>() );
    void resample( const std::vector<ci::fs::path> &filepaths, uint32_t resampling );
    void readGadgetAscii( const ci::fs::path &filepath );
    
    template <class Archive>
    void save( Archive & ar ) const;
    template <class Archive>
    void load( Archive & ar );
protected:
    ci::AxisAlignedBox3f    mBounds;
    ci::gl::VboMeshRef      mVbo;
    std::deque<Particle>    mParticles;
};



template <class Archive>
void Particle::serialize( Archive & ar )
{
    ar( mInitial, mFinal );
}

template <class Archive>
void ParticleSet::save( Archive & ar ) const
{
    ar( mParticles );
}

template <class Archive>
void ParticleSet::load( Archive & ar )
{
    ar( mParticles );
}
namespace cereal {
    
    template<class Archive>
    void save(Archive & archive,
              ci::vec3 const & m)
    {
        archive( m.x, m.y, m.z );
    }
    
    template<class Archive>
    void load(Archive & archive,
              ci::vec3 & m)
    {
        archive( m.x, m.y, m.z );
    }
    
}