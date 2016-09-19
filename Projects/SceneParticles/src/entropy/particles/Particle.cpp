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
		string meshName;
	};
	*/

	Particle::Data Particle::DATA[NUM_TYPES] = {
		{	0x01,	0,		0,		500.f,		-1.f,			{"Electron",		{0.5f, 0.5f, 0.5f, 0.5f}},	"sphere_electron_positron.obj"}, // ELECTRON
		{	~0x01,	0,		0,		500.f,		1.f,			{"Positron",		{0.0f, 0.1f, 0.5f, 0.5f}},	"sphere_electron_positron.obj"}, // POSITRON
		{	0x04,	0,		0,		2300.f,		-2.f / 3.f,		{"Anti up quark",	{0.2f, 0.2f, 0.2f, 0.5f}},	"particle_0.obj"              }, // ANTI_UP_QUARK
		{	~0x04,	~0x01,	0x01,	2300.f,		2.f / 3.f,		{"Up quark",		{0.1f, 0.1f, 0.1f, 0.5f}},	"particle_0.obj"              }, // UP_QUARK
		{	0x02,	0,		0,		4800.f,		1.f / 3.f,		{"Anti down quark", {0.0f, 0.1f, 0.5f, 0.5f}},	"particle_1.obj"              }, // ANTI_DOWN_QUARK
		{	~0x02,	0x01,	0x01,	4800.f,		-1.f / 3.f,		{"Down quark",		{0.5f, 0.5f, 0.5f, 0.5f}},	"particle_1.obj"              }, // DOWN_QUARK
		{	0,		0,		~0x01,	7100.f,		0.f,			{"Up down quark",	{0.0f, 0.1f, 0.5f, 0.5f}},	"particle_0_1.obj"			  }, // UP_DOWN_QUARK
		{	0,		0,		0,		14000.f,	0.f,			{"Neutron",			{0.2f, 0.2f, 0.2f, 0.5f}},	"particle_0_1_1.obj"              }, // NEUTRON
		{	0,		0,		0,		14000.f,	1.f,			{"Proton",			{0.0f, 0.1f, 0.5f, 0.5f}},	"particle_0_0_1.obj"			  }  // PROTON
	};

	ofParameterGroup Particle::parameters{
		"Particle",
		DATA[ELECTRON].color,
		DATA[POSITRON].color,
		DATA[ANTI_UP_QUARK].color,
		DATA[UP_QUARK].color,
		DATA[ANTI_DOWN_QUARK].color,
		DATA[DOWN_QUARK].color,
		DATA[UP_DOWN_QUARK].color,
		DATA[NEUTRON].color,
		DATA[PROTON].color
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
