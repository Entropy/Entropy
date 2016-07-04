/*
 *  ParticleSystem.cpp
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
#include "ParticleSystem.h"
#include "ofxObjLoader.h"

namespace nm
{
    ParticleSystem::ParticleSystem() :
        numParticles(0),
        roughness(.1f),
        particles(NULL),
        positions(NULL)
    {
    }
    
    ParticleSystem::~ParticleSystem()
    {
        if (particles) delete[] particles;
        if (positions) delete[] positions;
    }
    
    void ParticleSystem::init(const ofVec3f& min, const ofVec3f& max)
    {
        this->min = min;
        this->max = max;
        
        octree.init(min, max);
        octree.addChildren(true);
        
        particles = new nm::Particle[MAX_PARTICLES]();
        
        mesh = ofMesh::box(1.f, 1.f, 1.f, 1, 1, 1);
        
        shader.load("shaders/particle");
        
        // position stuff
        positions = new ParticleGpuData[MAX_PARTICLES]();
        tbo.allocate();
        tbo.bind(GL_TEXTURE_BUFFER);
        tbo.unbind(GL_TEXTURE_BUFFER);
        tbo.setData(sizeof(ParticleGpuData) * MAX_PARTICLES, positions, GL_DYNAMIC_DRAW);
        positionsTex.allocateAsBufferTexture(tbo, GL_RGBA32F);
    }
    
    void ParticleSystem::addParticle(const ofVec3f& position, const ofVec3f& velocity)
    {
        if (numParticles < MAX_PARTICLES)
        {
            particles[numParticles] = position;
            particles[numParticles].setVelocity(velocity);
            octree.addPoint(particles[numParticles]);
            numParticles++;
        }
        else ofLogError() << "Cannot add more particles";
    }
    
    void ParticleSystem::update()
    {
        octree.clear();
        /*if (octree.getChildren() != NULL)
        {
            for (unsigned i = 0; i < 8; ++i)
            {
                cout << i << ": " << octree.getChildren()[i].getCenterOfMass() << endl;
            }
        }*/
        octree.addPoints(particles, numParticles);
        octree.updateCenterOfCharge();
        
        float dt = ofGetLastFrameTime();
        tbb::parallel_for(tbb::blocked_range<size_t>(0, numParticles),
                          [&](const tbb::blocked_range<size_t>& r) {
                              for(size_t i = r.begin(); i != r.end(); ++i)
                              {
                                  particles[i].zeroForce();
                                  octree.sumForces(particles[i]);
                                  particles[i].addVelocity(particles[i].getForce() * dt / particles[i].getMass());
                                  //particles[i].addVelocity(dt * ofVec3f(1.f, 0.f, 0.f));
                                  particles[i] += particles[i].getVelocity() * dt;
                                  for (unsigned j = 0; j < 3; ++j)
                                  {
                                      if (particles[i][j] > max[j]) particles[i][j] = min[j];
                                      if (particles[i][j] < min[j]) particles[i][j] = max[j];
                                  }
                                  positions[i].transform =
                                      ofMatrix4x4::newLookAtMatrix(ofVec3f(0.0f, 0.0f, 0.0f), particles[i].getVelocity(), ofVec3f(0.0f, 1.0f, 0.0f)) *
                                      //ofMatrix4x4::newScaleMatrix(ofVec3f(particles[i].radius, particles[i].radius, particles[i].radius)) *
                                      ofMatrix4x4::newTranslationMatrix(particles[i]);
                              }
                          });
        tbo.updateData(0, sizeof(ParticleGpuData) * numParticles, positions);
    }
    
    void ParticleSystem::draw()
    {
        glPushAttrib(GL_ENABLE_BIT);
        glEnable( GL_DEPTH_TEST );
        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );
        shader.begin();
        {
            shader.setUniform1i("numLights", NUM_LIGHTS);
            shader.setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
            shader.setUniform1f("roughness", roughness);
            shader.setUniform3f("particleColor", 1.f, 1.f, 1.f);
            shader.setUniformTexture("uOffsetTex", positionsTex, 0);
            
            for (int i = 0; i < NUM_LIGHTS; i++)
            {
                string index = ofToString(i);
                shader.setUniform3f("lights[" + index + "].position", lightPosns[i] * ofGetCurrentViewMatrix());
                shader.setUniform4f("lights[" + index + "].color", lightCols[i]);
                shader.setUniform1f("lights[" + index + "].intensity", lightIntensities[i]);
                shader.setUniform1f("lights[" + index + "].radius", lightRadiuses[i]);
            }
            
            mesh.drawInstanced(OF_MESH_FILL, numParticles);
        }
        shader.end();
        glPopAttrib();
    }
}
