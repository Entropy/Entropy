/*
 *  Photons.cpp
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
#include "Photons.h"

namespace nm
{
	Photons::Photons() : 
		currentPhotonIdx(0)
	{
	}

	void Photons::init(Environment::Ptr env)
	{
		this->environment = env;

		// photon stuff
		posns.resize(MAX_PHOTONS);
		scaledPosns.resize(MAX_PHOTONS);
		vels.resize(MAX_PHOTONS);

		for (auto& p : posns) p = glm::vec3(numeric_limits<float>::max());
		for (auto& p : scaledPosns) p = glm::vec3(numeric_limits<float>::max());
		for (auto& v : vels) v = glm::sphericalRand(300.f);
		
		photonPosnBuffer.allocate();
		photonPosnBuffer.setData(sizeof(scaledPosns[0]) * MAX_PHOTONS, &scaledPosns[0].x, GL_DYNAMIC_DRAW);
		photonPosnTexture.allocateAsBufferTexture(photonPosnBuffer, GL_RGB32F);

		// particle stuff
		unsigned w = MAX_PHOTONS;
		unsigned h = PARTICLES_PER_PHOTON;

		trailParticles.init(w, h);

		if (ofIsGLProgrammableRenderer()) trailParticles.loadShaders("shaders/photon_update", "shaders/photon_draw");
		else ofLogError() << "Expected programmable renderer";

		// initial positions
		float* particlePosns = new float[w * h * 4];
		for (unsigned y = 0; y < h; ++y)
		{
			for (unsigned x = 0; x < w; ++x)
			{
				unsigned idx = y * w + x;
				particlePosns[idx * 4] = 1e10; // particle x (offscreen)
				particlePosns[idx * 4 + 1] = 1e10; // particle y (offscreen)
				particlePosns[idx * 4 + 2] = 0.f; // particle z
				particlePosns[idx * 4 + 3] = ofRandomuf(); // start age
			}
		}
		trailParticles.loadDataTexture(ofxGpuParticles::POSITION, particlePosns);
		delete[] particlePosns;

		// velocities will remain constant, only age will change
		float* particleVels = new float[w * h * 4];
		const float speed = 40.f;
		for (unsigned y = 0; y < h; ++y)
		{
			for (unsigned x = 0; x < w; ++x)
			{
				unsigned idx = y * w + x;
				particleVels[idx * 4] = ofRandom(-speed, speed); // vel x
				particleVels[idx * 4 + 1] = ofRandom(-speed, speed); // vel y
				particleVels[idx * 4 + 2] = ofRandom(-speed, speed); // vel z
				particleVels[idx * 4 + 3] = 0.f; // photon index
			}
		}
		trailParticles.loadDataTexture(ofxGpuParticles::VELOCITY, particleVels);
		delete[] particleVels;

		for (unsigned i = 0; i < trailParticles.getMeshRef().getNumVertices(); ++i)
		{
			//trailParticles.getMeshRef().addColor(ofFloatColor::fromHsb(ofRandom(.4f, .6f), .5f, 1.f)); // color
			trailParticles.getMeshRef().addColor(ofFloatColor(ofRandom(0.f, .2f), 0.f, 0.f)); // just put hue offset into the red channel
			trailParticles.getMeshRef().getColors().back().a = ofRandom(.1f, 1.f); // size
		}

		particleImage.load("images/particle1.png");

		// listen for ofxGpuParticle events to set additonal uniforms
		eventListeners.push_back(trailParticles.updateEvent.newListener([this](ofShader& shader)
		{
			ofVec3f mouse(ofGetMouseX() - .5f * ofGetWidth(), .5f * ofGetHeight() - ofGetMouseY(), 0.f);
			shader.setUniform3fv("mouse", mouse.getPtr());
			shader.setUniform1f("elapsedTime", ofGetElapsedTimef());
			shader.setUniform1f("frameTime", ofGetLastFrameTime());
			shader.setUniformTexture("photonPosnTexture", photonPosnTexture, 4);
		}));
		eventListeners.push_back(trailParticles.drawEvent.newListener([this](ofShader& shader)
		{
			shader.setUniformTexture("tex", particleImage, 4);
			shader.setUniform1f("energy", environment->getEnergy());
		}));

		// listen for dead particles events
//		eventListeners.push_back(environment->deadParticlesEvent.newListener([this](DeadParticlesEventArgs& args)
//		{
//			for (int i = 0; i < args.numDead; ++i)
//			{
//				tryPairProduction();
//			}
//		}));

		// listen for photon events
		eventListeners.push_back(environment->photonEvent.newListener([this](PhotonEventArgs& args)
		{
			for (unsigned i = 0; i < args.numPhotons; ++i)
			{
				currentPhotonIdx = (currentPhotonIdx + 1) % posns.size();
				posns[currentPhotonIdx] = args.photons[i];
			}
		}));
	}

	void Photons::update(double dt)
	{
		dt *= environment->systemSpeed;
		const glm::vec3 min = environment->getMin();
		const glm::vec3 max = environment->getMax();
		const float expansionScalar = environment->getExpansionScalar();

		for (unsigned i = 0; i < posns.size(); ++i)
		{
			if (posns[i].x != numeric_limits<float>::max())
			{
				posns[i] += vels[i] * dt;
				// check whether photon is out of bounds
				for (unsigned j = 0; j < 3; ++j)
				{
					if (posns[i][j] > max[j]) posns[i][j] = min[j];
					if (posns[i][j] < min[j]) posns[i][j] = max[j];
				}
				scaledPosns[i] = expansionScalar * posns[i];
			}
			else scaledPosns[i] = posns[i];
		}
		photonPosnBuffer.updateData(0, sizeof(scaledPosns[0]) * scaledPosns.size(), &scaledPosns[0].x);
		trailParticles.update();
	}

//	bool Photons::tryPairProduction()
//	{
//		if (ofRandomuf() < environment->getPairProductionThresh())
//		{
//			// find a particle
//			const unsigned numTries = 100;
//			unsigned idx = numeric_limits<unsigned>::max();
//			for (unsigned i = 0; i < numTries; ++i)
//			{
//				unsigned randIdx = rand() % MAX_PHOTONS;
//				if (posns[randIdx].x != numeric_limits<float>::max())
//				{
//					idx = randIdx;
//					break;
//				}
//			}
//			if (idx != numeric_limits<unsigned>::max())
//			{
//				PairProductionEventArgs args;
//				args.position = posns[idx];
//				args.velocity = vels[idx];
//				ofNotifyEvent(environment->pairProductionEvent, args, this);
//				posns[idx] = glm::vec3(numeric_limits<float>::max());

//				return true;
//			}
//		}
//		return false;
//	}

	void Photons::draw()
	{
		ofEnablePointSprites();

		trailParticles.draw();

		ofDisablePointSprites();
		ofDisableBlendMode();
	}
}
