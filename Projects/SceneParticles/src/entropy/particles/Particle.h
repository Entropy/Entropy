/*
 *  Particle.h
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

namespace nm
{
	class Particle : public glm::vec3
	{
	public:
		enum Type
		{
			ELECTRON,
			POSITRON,

			ANTI_UP_QUARK,
			UP_QUARK,

			ANTI_DOWN_QUARK,
			DOWN_QUARK,
			
			UP_DOWN_QUARK, // hacky composite particle
			
			NEUTRON,
			PROTON,

			NUM_TYPES
		};

		struct Data
		{
            char annihilationFlag; // particles and anti-particles
            char fusion1Flag; // up-quarks and down-quarks
            char fusion2Flag; // compound particles with up-quarks and down-quarks
			float mass;
			float charge;
			ofFloatColor color;
			string meshName;
		};

		static const Data DATA[NUM_TYPES];

		Particle();

		inline void zeroForce() { force = glm::vec3(0.0f); }
		inline glm::vec3 getForce() const { return force; }
		inline void setForce(const glm::vec3& force) { this->force = force; }

		inline void setMass(float mass) { this->mass = mass; }
		inline float getMass() const { return mass; }

		inline void setCharge(float charge) { this->charge = charge; }
		inline float getCharge() const { return charge; }

		inline void setPosition(const glm::vec3& position) { this->x = position.x; this->y = position.y; this->z = position.z; }
		inline void setVelocity(const glm::vec3& velocity) { this->velocity = velocity; }
		inline glm::vec3 getVelocity() const { return velocity; }

		inline void setRadius(float radius) { this->radius = radius; }
		inline float getRadius() const { return radius; }

		inline void setType(Particle::Type type) { this->type = type; }
		inline Type getType() const { return type; }

		inline unsigned char getAnnihilationFlag() const { return DATA[type].annihilationFlag; }
		inline unsigned char getFusion1Flag() const { return DATA[type].fusion1Flag; }
		inline unsigned char getFusion2Flag() const { return DATA[type].fusion2Flag; }
        
    private:
        Type type;
        float mass;
        float charge;
		glm::vec3 velocity;
		glm::vec3 force;
		float radius;
    };
}
