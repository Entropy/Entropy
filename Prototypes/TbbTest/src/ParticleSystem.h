/*
 *  ParticleSystem.h
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
#pragma once

#include "Octree.h"
#include "Particle.h"

namespace nm
{
    struct ParticleGpuData
    {
        ofMatrix4x4 transform;
    };
    
    class ParticleSystem
    {
    public:
        static const unsigned MAX_PARTICLES = 5000;
        static const unsigned NUM_LIGHTS = 2;
        
        ParticleSystem();
        ~ParticleSystem();
        
        void init(const ofVec3f& min, const ofVec3f& max);
        
        void addParticle(Particle::Type type, const ofVec3f& position, const ofVec3f& velocity);
        
        void update();
        
        void draw();
        
        // lighting, should be private but for
        // GUI adding simplicity they're public
        ofVec3f lightPosns[NUM_LIGHTS];
        ofFloatColor lightCols[NUM_LIGHTS];
        float lightIntensities[NUM_LIGHTS];
        float lightRadiuses[NUM_LIGHTS];
        float roughness;
        
    private:
        void sumForces(Particle& particle);
        
        Octree<Particle> octree;
        nm::Particle* particles;
        unsigned numParticles[Particle::NUM_TYPES];
		unsigned totalNumParticles;
        ofVboMesh meshes[Particle::NUM_TYPES];
        ofShader shader;
        ofVec3f min, max;
        
        // position stuff
        ofBufferObject tbo[Particle::NUM_TYPES];
		ParticleGpuData* positions[Particle::NUM_TYPES];
        ofTexture positionsTex[Particle::NUM_TYPES];
    };
}
