/*
 *  Environment.h
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
	struct PhotonEventArgs
	{
		glm::vec3* photons;
		unsigned numPhotons;
	};

	struct PairProductionEventArgs
	{
		glm::vec3 position;
		glm::vec3 velocity;
	};

	class Environment
	{
	public:
		typedef shared_ptr<Environment> Ptr;

		Environment(const glm::vec3& min, const glm::vec3& max);

		inline glm::vec3 getMin() const { return min; }
		inline glm::vec3 getMax() const { return max; }
		inline glm::vec3 getDims() const { return dims; }

		inline float getEnergy() const { return energy; }
		inline void setEnergy(float energy) { this->energy = energy; }
		inline float& getEnergyRef() { return energy; } // for GUI

		float getExpansionScalar() const;

		float getForceMultiplier() const;

		ofEvent<PairProductionEventArgs> pairProductionEvent;
		ofEvent<PhotonEventArgs> photonEvent;

	private:
		glm::vec3 min, max, dims;
		float energy; // from 0 to 1
		float forceMultiplierMin, forceMultiplierMax;
    };
}
