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

	struct DeadParticlesEventArgs
	{
		uint32_t numDead;
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
		enum State{
			BARYOGENESIS,
			STANDARD_MODEL,
			NUCLEOSYNTHESIS,
		};

		Environment(const glm::vec3& min, const glm::vec3& max);

		void update();

		inline glm::vec3 getMin() const { return min; }
		inline glm::vec3 getMax() const { return max; }
		inline glm::vec3 getDims() const { return dims; }

		inline float getEnergy() const { return energy; }
		inline void setEnergy(float energy) { this->energy = energy; }

		float getExpansionScalar() const;
		float getForceMultiplier() const;
		float getAnnihilationThresh() const;
		float getFusionThresh() const;
		float getPairProductionThresh() const;

		ofEvent<PairProductionEventArgs> pairProductionEvent;
		ofEvent<PhotonEventArgs> photonEvent;
		ofEvent<DeadParticlesEventArgs> deadParticlesEvent;

		ofParameter<int> state{"state", 0, 0, 2};
		ofParameter<string> stateStr{"BARYOGENESIS"};
		ofParameter<float> energy{ "Energy", 1.0f, 0.0f, 1.0f };
		ofParameter<float> forceMultiplierMin{ "Force Multiplier Min", 1e7, 1e7, 1e8 };
		ofParameter<float> forceMultiplierMax{ "Force Multiplier Max", 1e8, 1e7, 1e8 };
		ofParameter<float> annihilationThreshMin{ "Annihiliation Threshold Min", 0.3f, 0.0f, 1.0f };
		ofParameter<float> annihilationThreshMax{ "Annihiliation Threshold Max", 0.5f, 0.0f, 1.0f };
		ofParameter<float> fusionThresholdExponentMin{ "Fusion Threshold Exponent Min", 0.3f, 0.0f, 1.0f}; //-5.237f, -6.0f, -5.0f };
		ofParameter<float> fusionThresholdExponentMax{ "Fusion Threshold Exponent Max", 0.5f, 0.0f, 1.0f };// -5.593, -6.0f, -5.0f };
		ofParameter<float> pairProductionThresholdMin{ "Pair Production Threshold Min", 0.428f, 0.0f, 1.0f };
		ofParameter<float> pairProductionThresholdMax{ "Pair Production Threshold Max", 0.572f, 0.0f, 1.0f };
		ofParameter<float> quarkCreationChance{"quarkCreationChance", 0.2, 0, 1};
		ofParameter<float> matterSurveivesChance{"Matter survives chance", 0.5, 0, 1};
		ofParameter<float> systemSpeed{ "System speed", 0.5, 0, 3, ofParameterScale::Logarithmic};


		ofParameterGroup parameters{
			"Environment",
			state,
			stateStr,
			systemSpeed,
			energy,
			forceMultiplierMin,
			forceMultiplierMax,
			annihilationThreshMin,
			annihilationThreshMax,
			fusionThresholdExponentMin,
			fusionThresholdExponentMax,
			quarkCreationChance,
			matterSurveivesChance,
			pairProductionThresholdMin,
			pairProductionThresholdMax
		};
	private:
		glm::vec3 min, max, dims;
    };
}
