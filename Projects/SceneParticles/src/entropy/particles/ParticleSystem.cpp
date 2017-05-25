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
	const float ParticleSystem::MIN_SPEED_SQUARED = 1;
	const float ParticleSystem::MAX_SPEED = 1e14;
	const float ParticleSystem::MAX_SPEED_SQUARED = MAX_SPEED * MAX_SPEED;

	ParticleSystem::ParticleSystem() :
		totalNumParticles(0),
		roughness(.1f),
		numDeadParticles(0),
		numNewPhotons(0)
	{
		this->clearParticles();
	}

	void ParticleSystem::init(Environment::Ptr universe)
	{
		this->environment = universe;

		octree.init(universe->getMin(), universe->getMax());
		octree.addChildren(true);

		particles.fill(Particle());
		int i = 0;
		for(auto &p : particles){
			p.id = i++;
		}
		deadParticles.fill(0);
		newPhotons.fill(glm::vec3(0));

		for (unsigned i = 0; i < Particle::NUM_TYPES; ++i)
		{
			ostringstream oss;
			oss << "models/";
			oss << Particle::DATA[i].meshName;
            ofxObjLoader::load(oss.str(), meshes[i]);
			//meshesFill[i] = ofSpherePrimitive(1.0f, 3).getMesh();
            /*if (i%2 == 0)
				meshesWire[i] = ofIcoSpherePrimitive(1.0f, 1).getMesh();
			else
                meshesWire[i] = ofSpherePrimitive(1.0f, 6).getMesh();*/

            meshes[i].setUsage(GL_STATIC_DRAW);
		}

		wallShader.load("shaders/wall");

		// position stuff
		for (unsigned i = 0; i < Particle::NUM_TYPES; ++i)
		{
			positions[i].fill(ParticleGpuData());
			tbo[i].allocate();
			tbo[i].setData(sizeof(ParticleGpuData) * MAX_PARTICLES, positions.data(), GL_DYNAMIC_DRAW);
			positionsTex[i].allocateAsBufferTexture(tbo[i], GL_RGBA32F);
		}

		pairProductionListener = universe->pairProductionEvent.newListener([this](PairProductionEventArgs & args)
		{
			Particle::Type type1, type2;
			switch (rand() % 3)
			{
			case 0:
				type1 = Particle::UP_QUARK;
				if(ofRandomuf()<environment->matterSurveivesChance){
					type2 = Particle::ANTI_UP_QUARK;
				}else{
					type2 = Particle::UP_QUARK;
				}
				break;

			case 1:
				type1 = Particle::DOWN_QUARK;
				 if(ofRandomuf()<environment->matterSurveivesChance){
					 type2 = Particle::ANTI_DOWN_QUARK;
				 }else{
					 type2 = Particle::DOWN_QUARK;
				 }
				break;

			case 2:
				type1 = Particle::ELECTRON;
				if(ofRandomuf()<environment->matterSurveivesChance){
					type2 = Particle::POSITRON;
				}else{
					type2 = Particle::ELECTRON;
				}
				break;
			}
			glm::vec3 dir = glm::normalize(glm::perp(args.velocity, glm::sphericalRand(1.f)));
			float speed = glm::length(args.velocity);
			addParticle(type1, args.position, .5f * speed * dir);
			addParticle(type2, args.position, -.5f * speed * dir);
		});
	}

	void ParticleSystem::addParticle(Particle::Type type, const glm::vec3& position, const glm::vec3& velocity)
	{
		if (totalNumParticles < MAX_PARTICLES)
		{
			float radius = ofMap(Particle::DATA[type].mass, 500.f, 2300.f, 5.0f, 8.0f);

			Particle& p = particles[totalNumParticles];
			p.setPosition(position);
			p.setVelocity(velocity);
			p.setType(type);
			p.setCharge(Particle::DATA[type].charge);
			p.setMass(Particle::DATA[type].mass);
			p.setRadius(radius);
			p.anihilationRatio = 0;
			p.alive = true;

			totalNumParticles++;
			numParticles[type]++;
		}
		else ofLogError() << "Cannot add more particles";
	}

	void ParticleSystem::clearParticles()
	{
		totalNumParticles = 0;
		for(auto & pos: positions){
			pos.fill(ParticleGpuData());
		}
		for(auto & p: particles){
			p.alive = false;
		}
		numParticles.fill(0);
	}

	void ParticleSystem::update(double dt)
	{
		if(dt == 0) return;
		dt *= environment->systemSpeed;
		numDeadParticles = 0;
		numNewPhotons = 0;
		octree.clear();
		octree.addPoints(particles.data(), totalNumParticles);
		octree.updateCenterOfCharge();

		tbb::atomic<unsigned> typeIndices[Particle::NUM_TYPES];
		for (unsigned i = 0; i < Particle::NUM_TYPES; ++i) typeIndices[i] = 0;
		const glm::vec3 min = environment->getMin();
		const glm::vec3 max = environment->getMax();
		const float expansionScalar = environment->getExpansionScalar();
		const float annihilationThreshold = environment->getAnnihilationThresh(); // was 0.5
		const float fusionThreshold = environment->getFusionThresh(); // was 0.00001
		Octree<Particle>::setForceMultiplier(environment->getForceMultiplier());
		//const float forceMultiplier = universe->getForceMultiplier();
		tbb::parallel_for(tbb::blocked_range<size_t>(0, totalNumParticles),
			[&](const tbb::blocked_range<size_t>& r) {
			for (size_t i = r.begin(); i != r.end(); ++i)
			{
				///////////////////////////////////////////////////////////
				// attraction/repulsion
				///////////////////////////////////////////////////////////

				// zero particle forces
				particles[i].zeroForce();

				// sum all forces acting on particle
				particles[i].potentialInteractionPartners.clear();

				if(!particles[i].alive) continue;

				octree.sumForces(particles[i]);// , forceMultiplier);

				// add velocity (TODO: improved Euler integration)
				particles[i].setVelocity(particles[i].getVelocity() + particles[i].getForce() * dt / particles[i].getMass());

				// damp velocity
				float velocity2 = glm::length2(particles[i].getVelocity());
				if (velocity2 > MIN_SPEED_SQUARED) particles[i].setVelocity(.998f * particles[i].getVelocity());
				if (velocity2 > MAX_SPEED_SQUARED) particles[i].setVelocity(MAX_SPEED * glm::normalize(particles[i].getVelocity()));

				// add position (TODO: improved Euler integration)
				particles[i].pos += particles[i].getVelocity() * dt;
				if(particles[i].potentialInteractionPartners.empty()){
					particles[i].anihilationRatio = 0;
					particles[i].fusionRatio = 1;
				}

				// check whether particle is out of bounds
				for (unsigned j = 0; j < 3; ++j)
				{
					// add a little bit so things don't get stuck teleporting on the edges
					if (particles[i].pos[j] > max[j]) particles[i].pos[j] = min[j] + 10.f;
					if (particles[i].pos[j] < min[j]) particles[i].pos[j] = max[j] - 10.f;
				}

				unsigned idx = typeIndices[particles[i].getType()].fetch_and_increment();
				positions[particles[i].getType()][idx].transform =
					glm::translate(expansionScalar * particles[i].pos) *
					glm::scale(glm::vec3(particles[i].getRadius())) *
					glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), particles[i].getVelocity(), glm::vec3(0.0f, 1.0f, 0.0f));

				/////////////////////////////////////////////////////////////
				// particle interactions
				/////////////////////////////////////////////////////////////

				bool killParticles = false;
				for(auto * potentialInteractionPartner: particles[i].potentialInteractionPartners){
					// make the particle with the lower address in memory
					// the one that is responsible for the interaction

//					if(potentialInteractionPartner){
//						float distance = glm::distance(potentialInteractionPartner->pos, particles[i].pos);
//						if(distance < Octree<Particle>::INTERACTION_DISTANCE()){
//							particles[i].pos += (potentialInteractionPartner->pos - particles[i].pos)*0.01;
//						}
//					}
					if (potentialInteractionPartner && particles[i].id < potentialInteractionPartner->id)
					{
						float distance = glm::distance(potentialInteractionPartner->pos, particles[i].pos);
						if(distance < Octree<Particle>::INTERACTION_DISTANCE()){
							// have this so later on can have different likelihoods of different
							// interactions occurring

							// see what type of interaction we have
							// already doing this check in sumForces() so maybe could remove to optimize but
							// doesn't seem like it would save a worthwhile amount of time
							if ((potentialInteractionPartner->getAnnihilationFlag() ^ particles[i].getAnnihilationFlag()) == 0xFF)
							{
								particles[i].anihilationRatio = particles[i].anihilationRatio + dt;// / environment->systemSpeed;
								if (particles[i].anihilationRatio > annihilationThreshold)
								{
									unsigned newPhotonIdx = numNewPhotons.fetch_and_increment();
									newPhotons[newPhotonIdx] = (particles[i].pos + potentialInteractionPartner->pos) / 2.f;
									killParticles = true;
								}
							}

							if (killParticles)
							{
								// Super hack to simulate matter surviving anihilation
								if(ofRandomuf()<environment->matterSurveivesChance){
									if(particles[i].isAntiMatterQuark()){
										deadParticles[numDeadParticles.fetch_and_increment()] = i;
										particles[i].alive = false;
									}else{
										// If matter survives teleport it to the other side of the universe
										// so we can't see it
										particles[i].pos = -particles[i].pos;
										particles[i].potentialInteractionPartners.clear();
									}

									deadParticles[numDeadParticles.fetch_and_increment()] = potentialInteractionPartner - particles.data();
									potentialInteractionPartner->alive = false;
								}else{
									// interaction is annihilation so kill particle
									deadParticles[numDeadParticles.fetch_and_increment()] = i;
									particles[i].alive = false;

									// kill partner (idx of partner is potentialInteractionPartner - particles)
									deadParticles[numDeadParticles.fetch_and_increment()] = potentialInteractionPartner - particles.data();
									potentialInteractionPartner->alive = false;
								}

								// The particle is probably dead, don't look for more interactions;
								break;
							}
						}
					}
				}



				if(!killParticles && environment->state > nm::Environment::BARYOGENESIS){
					auto type = particles[i].getType();
					if(type==Particle::DOWN_QUARK || type==Particle::UP_QUARK){
						const auto & partners = particles[i].potentialInteractionPartners;
						auto down = std::find_if(partners.begin(), partners.end(), [&](const Particle * p){
							return p->getType() == Particle::DOWN_QUARK && p->alive;
						});
						auto up = std::find_if(partners.begin(), partners.end(), [&](const Particle * p){
							return p->getType() == Particle::UP_QUARK && p->alive;
						});
						if (up != partners.end() && down != partners.end())
						{
							particles[i].fusionRatio = particles[i].fusionRatio + dt;
							if (particles[i].fusionRatio > fusionThreshold)
							{
								Particle::Type newType = Particle::PROTON;
								if (particles[i].getType() == Particle::DOWN_QUARK)
								{
									newType = Particle::NEUTRON;
								}
								addParticle(
									newType,
									particles[i].pos,
									((*up)->getVelocity() + (*down)->getVelocity() + particles[i].getVelocity()) / 3.f
								);
								killParticles = true;
							}
						}

						if(killParticles){
							// interaction is fusion so kill particle
							deadParticles[numDeadParticles.fetch_and_increment()] = i;
							particles[i].alive = false;

							// kill partners (idx of partner is partner - particles)
							deadParticles[numDeadParticles.fetch_and_increment()] = *up - particles.data();
							(*up)->alive = false;

							deadParticles[numDeadParticles.fetch_and_increment()] = *down - particles.data();
							(*down)->alive = false;
						}
					}
				}
			}
		});

		if (numDeadParticles > 0)
		{
			DeadParticlesEventArgs args;
			args.numDead = numDeadParticles;
			ofNotifyEvent(environment->deadParticlesEvent, args, this);
		}

		// start deleting particles at the end first so we don't swap out a particle
		// that is actually dead and would be swapped out later in the iteration
		std::sort(deadParticles.begin(), deadParticles.begin() + numDeadParticles, std::greater<float>());

		//cout << "Num dead particles is " << numDeadParticles << " / " << totalNumParticles << endl;
		if (numDeadParticles)
		{
			// kill all the particles
			for (unsigned i = 0; i < numDeadParticles; ++i)
			{
				unsigned deadIdx = deadParticles[i];
				unsigned endIdx = totalNumParticles.fetch_and_decrement() - 1;
				const auto idx = (unsigned)particles[deadIdx].getType();
				//cout << "  Killing particle " << i << ": From " << numParticles[idx];
				numParticles[idx].fetch_and_decrement();
				//cout << " to " << numParticles[idx] << endl;
				// replace dead particle with one from the end of the array
				if (endIdx >= 0 && deadIdx < totalNumParticles) std::swap(particles[deadIdx], particles[endIdx]);
			}

//			auto lastAlive = std::remove_if(particles.begin(), particles.begin() + totalNumParticles, [&](Particle & p){
//				if(!p.alive){
//					numParticles[p.getType()].fetch_and_decrement();
//				}
//				return !p.alive;
//			});

//			totalNumParticles = lastAlive - particles.begin();
		}

		if (numNewPhotons)
		{
			// notify photon listeners
			PhotonEventArgs photonEventArgs;
			photonEventArgs.photons = newPhotons.data();
			photonEventArgs.numPhotons = numNewPhotons;
			ofNotifyEvent(environment->photonEvent, photonEventArgs, this);
		}

		// update the texture buffer objects with the new positions of particles
		for (unsigned i = 0; i < Particle::NUM_TYPES; ++i)
		{
			//cout << "Updating " << i << " w/ " << numParticles[i] << " particles" << endl;
			tbo[i].updateData(0, sizeof(ParticleGpuData) * numParticles[i], positions[i].data());
		}



		std::array<unsigned,Particle::NUM_TYPES> typeIndicesCheck;
		typeIndicesCheck.fill(0);
		for(auto i = particles.begin(); i<particles.begin() + totalNumParticles; ++i){
			auto & p = *i;
			typeIndicesCheck[p.getType()]++;
		}
		for(size_t i=0;i<typeIndicesCheck.size();i++){
			if(numParticles[i] != typeIndicesCheck[i]){
				ofLogError() << "num particles for type " << i << " doesn' match " << " class " << numParticles[i] << " check " << typeIndicesCheck[i] << endl;
			}
		}
		auto wrongAlive = std::count_if(particles.begin() + totalNumParticles, particles.end(), [&](Particle & p){
			return (bool)p.alive;
		});
		if(wrongAlive>0){
			ofLogError() << wrongAlive << " alive particles after dead zone";
		}
		auto wrongDead = std::count_if(particles.begin(), particles.begin() + totalNumParticles, [&](Particle & p){
			return !p.alive;
		});
		if(wrongDead>0){
			ofLogError() << wrongDead << " dead particles before dead zone";
		}
		auto totalFromTypes = std::accumulate(numParticles.begin(), numParticles.end(), 0, [&](int acc, unsigned num){
			return acc + num;
		});
		if(totalFromTypes!=totalNumParticles){
			ofLogError() << "total from types " << totalFromTypes << " != " << totalNumParticles;
		}

	}

    void ParticleSystem::draw(ofShader & shader)
	{
		for (unsigned i = 0; i < Particle::NUM_TYPES; ++i)
		{
			if (numParticles[i])
            {
                shader.setUniform1f("uScale", 1.0f);
                shader.setUniform1f("uType", i);
                shader.setUniformTexture("uOffsetTex", positionsTex[i], 0);
                meshes[i].drawInstanced(OF_MESH_FILL, numParticles[i]);
            }
		}
	}

	void ParticleSystem::drawWalls()
	{
		// Save state before changing.
		auto depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
		auto cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
		GLint cullFaceMode[1];
		glGetIntegerv(GL_CULL_FACE_MODE, cullFaceMode);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		ofEnableDepthTest();

		wallShader.begin();
		{
			wallShader.setUniform1i("numLights", NUM_LIGHTS);
			wallShader.setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
			wallShader.setUniform1f("roughness", roughness);

			for (int i = 0; i < NUM_LIGHTS; i++)
			{
				string index = ofToString(i);
                wallShader.setUniform3f("lights[" + index + "].position", (ofGetCurrentViewMatrix() * glm::vec4(lights[i].position, 0.0f)).xyz());
				wallShader.setUniform4f("lights[" + index + "].color", lights[i].color);
				wallShader.setUniform1f("lights[" + index + "].intensity", lights[i].intensity);
				wallShader.setUniform1f("lights[" + index + "].radius", lights[i].radius);
			}
			wallShader.setUniform3f("wallColor", 1.f, 1.f, 1.f);

			const float expansionScalar = environment->getExpansionScalar();
			const glm::vec3 min = expansionScalar * environment->getMin();
			const glm::vec3 max = expansionScalar * environment->getMax();
			const glm::vec3 dims = expansionScalar * environment->getDims();

			// stop using ofDrawBox for walls and change it to one mesh
			ofDrawBox(0.f, 0.f, min.z - 5.f, dims.x, dims.y, 10.f); // back wall

			ofDrawBox(0.f, min.y - 5.f, 0.f, dims.x, 10.f, dims.z); // floor
			ofDrawBox(0.f, max.y + 5.f, 0.f, dims.x, 10.f, dims.z); // ceiling

			ofDrawBox(min.x - 5.f, 0.f, 0.f, 10.f, dims.y, dims.z); // left wall
			ofDrawBox(max.x + 5.f, 0.f, 0.f, 10.f, dims.y, dims.z); // right wall
		}
		wallShader.end();

		// Restore state.
		glCullFace(cullFaceMode[0]);
		if (depthTestEnabled) ofDisableDepthTest();
		if (!cullFaceEnabled) glDisable(GL_CULL_FACE);
	}

	//--------------------------------------------------------------
	void ParticleSystem::serialize(nlohmann::json & json)
	{
		auto & jsonGroup = json["particles"];
		for (int i = 0; i < totalNumParticles; ++i)
		{
			nlohmann::json jsonParticle;
			Particle & p = particles[i];

			jsonParticle["position"] = ofToString(p.pos);
			jsonParticle["type"] = p.getType();
			//jsonParticle["mass"] = p.getMass();
			//jsonParticle["charge"] = p.getCharge();
			//jsonParticle["radius"] = p.getRadius();
			jsonParticle["velocity"] = ofToString(p.getVelocity());
			jsonParticle["force"] = ofToString(p.getForce());
			
			jsonGroup.push_back(jsonParticle);
		}
	}

	//--------------------------------------------------------------
	void ParticleSystem::deserialize(const nlohmann::json & json)
	{
		if (json.count("particles"))
		{
			// Reset counts.
			totalNumParticles = 0;
			numParticles.fill(0);

			auto & jsonGroup = json["particles"];
			for (int i = 0; i < jsonGroup.size(); ++i)
			{
				const auto & jsonParticle = jsonGroup[i];

				// Set the force first.
				Particle & p = particles[totalNumParticles];
				p.setForce(ofFromString<glm::vec3>(jsonParticle["force"]));

				// Set the remaining attributes.
				const auto type = (Particle::Type)(int)jsonParticle["type"];
				const auto position = ofFromString<glm::vec3>(jsonParticle["position"]);
				const auto velocity = ofFromString<glm::vec3>(jsonParticle["velocity"]);
				addParticle(type, position, velocity);
			}
		}
	}


	//--------------------------------------------------------------
	gsl::span<nm::Particle> ParticleSystem::getParticles(){
		return gsl::span<nm::Particle>(particles.begin(), particles.begin() + totalNumParticles);
	}

	//--------------------------------------------------------------
	std::vector<Particle*> ParticleSystem::findNearestThan(const Particle & p, float distance) const{
		std::vector<Particle*> nearList;
		octree.findNearestThan(p, distance, nearList);
		return nearList;
	}

	//--------------------------------------------------------------
	std::vector<Particle*> ParticleSystem::findNearestThanByType(const Particle & p, float distance, std::initializer_list<Particle::Type> allowedTypes) const{
		std::vector<Particle*> nearList;
		octree.findNearestThanByType(p, distance, allowedTypes, nearList);
		return nearList;
	}

	nm::Particle * ParticleSystem::getById(size_t id){
		for(auto & p: getParticles()){
			if(p.id == id){
				return &p;
			}
		}
		return nullptr;
	}
}
