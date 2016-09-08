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

#include "ofMain.h"
#include "Octree.h"
#include "Particle.h"
#include "Environment.h"

namespace nm
{
	struct ParticleGpuData
	{
		glm::mat4 transform;
	};

	struct Light
	{
		glm::vec3 position;
		ofFloatColor color;
		float intensity;
		float radius;
	};

	class ParticleSystem
	{
	public:
		static const unsigned MAX_PARTICLES = 5000;
		static const unsigned NUM_LIGHTS = 2;
		static const float MIN_SPEED_SQUARED;
		static const float MAX_SPEED;
		static const float MAX_SPEED_SQUARED;

		ParticleSystem();

		void init(Environment::Ptr environment);

		void addParticle(Particle::Type type, const glm::vec3& position, const glm::vec3& velocity);
		void clearParticles();

		void update();

        void draw(ofShader & shader);

		void drawWalls();

		// lighting, should be private but for
		// GUI adding simplicity they're public
		Light lights[NUM_LIGHTS];
		float roughness;

		void serialize(nlohmann::json & json);
		void deserialize(const nlohmann::json & json);

	private:
		void onPairProduction(PairProductionEventArgs& args);

		Environment::Ptr environment;
		Octree<Particle> octree;

		std::array<nm::Particle, MAX_PARTICLES> particles;
		std::array<tbb::atomic<unsigned>, Particle::NUM_TYPES> numParticles;
		tbb::atomic<unsigned> totalNumParticles;

		std::array<unsigned, MAX_PARTICLES> deadParticles;
		tbb::atomic<unsigned> numDeadParticles;

		std::array<ofVboMesh, Particle::NUM_TYPES> meshes;
		ofShader wallShader;

		// position stuff
		std::array<ofBufferObject, Particle::NUM_TYPES> tbo;
		std::array<std::array<ParticleGpuData, MAX_PARTICLES>, Particle::NUM_TYPES> positions;
		std::array<ofTexture, Particle::NUM_TYPES> positionsTex;

		std::array<glm::vec3, MAX_PARTICLES> newPhotons;
		tbb::atomic<unsigned> numNewPhotons;
	};
}
