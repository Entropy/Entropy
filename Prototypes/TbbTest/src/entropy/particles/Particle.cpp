/*
 *  Particle.cpp
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
#include "Particle.h"

namespace nm
{
	/*
	struct Data
	{
		unsigned char annihilationFlag;
		unsigned char fusion1Flag;
		unsigned char fusion2Flag;
		float mass;
		float charge;
		ofFloatColor color;
	};
	*/

	const Particle::Data Particle::DATA[NUM_TYPES] = {
		{	0x01,	0,		0,		500.f,		-1.f,			ofFloatColor(0.f)	}, // ELECTRON
		{	~0x01,	0,		0,		500.f,		1.f,			ofFloatColor(1.f)	}, // POSITRON
		{	0x04,	0,		0,		2300.f,		-2.f / 3.f,		ofFloatColor(0.f)	}, // ANTI_UP_QUARK
		{	~0x04,	~0x01,	0x01,	2300.f,		2.f / 3.f,		ofFloatColor(1.f)	}, // UP_QUARK
		{	0x02,	0,		0,		4800.f,		1.f / 3.f,		ofFloatColor(1.f)	}, // ANTI_DOWN_QUARK
		{	~0x02,	0x01,	0x01,	4800.f,		-1.f / 3.f,		ofFloatColor(0.f)	}, // DOWN_QUARK
		{	0,		0,		~0x01,	7100.f,		1.f / 3.f,		ofFloatColor(1.f)	}  // UP_DOWN_QUARK
	};

    Particle::Particle() :
        glm::vec3(0.0f),
        mass(1.f),
        velocity(1.f, 0.f, 0.f),
        charge(1.f),
		radius(10.f)
    {
    }
}
