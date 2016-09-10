#include "Bursts.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace bubbles
	{
		//--------------------------------------------------------------
		Bursts::Bursts()
		{}

		//--------------------------------------------------------------
		Bursts::~Bursts()
		{
			this->exit();
		}

		//--------------------------------------------------------------
		void Bursts::init()
		{
			this->shader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/burst.vert");
			this->shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/burst.frag");
			this->shader.bindAttribute(ExtraAttribute::Age, "age");
			this->shader.bindDefaults();
			this->shader.linkProgram();
			
			// Add parameter listeners.
			this->parameterListeners.push_back(this->resolution.newListener([this](int &)
			{
				this->unitSphere.clear();
			}));
		}

		//--------------------------------------------------------------
		void Bursts::exit()
		{
			this->reset();

			this->unitSphere.clear();
			this->vbo.clear();

			this->parameterListeners.clear();
		}

		//--------------------------------------------------------------
		void Bursts::reset()
		{
			this->pos.clear();
			this->vel.clear();
			this->acc.clear();

			this->age.clear();
			this->links.clear();

			this->zombies.clear();
		}

		//--------------------------------------------------------------
		void Bursts::addDrop(const glm::vec3 & center, float radius)
		{
			// Build unit sphere if necessary.
			if (this->unitSphere.getNumVertices() == 0)
			{
				this->unitSphere = ofMesh::sphere(1.0f, this->resolution);
			}

			for (auto & vert : this->unitSphere.getVertices())
			{
				size_t idx;
				if (this->zombies.empty())
				{
					// Add a new particle at the end of the list.
					idx = this->pos.size();
					this->pos.resize(idx + 1);
					this->vel.resize(idx + 1);
					this->acc.resize(idx + 1);
					this->age.resize(idx + 1);
					this->links.resize(idx + 1);
				}
				else
				{
					// Reuse a zombie particle.
					idx = this->zombies.back();
					this->zombies.pop_back();
				}

				// Set initial values for the drop.
				this->pos[idx] = glm::vec3(center + vert * radius * 2.0f);
				this->vel[idx] = glm::vec3(0.0f);
				// Force is based on world size.
				const auto force = this->worldBounds * ofRandom(this->forceMultiplier - this->forceRandom, this->forceMultiplier + this->forceRandom);
				this->acc[idx] = glm::vec3(vert * force);

				this->age[idx] = this->maxAge;
			}
		}

		//--------------------------------------------------------------
		void Bursts::update(double dt)
		{			
#pragma omp parallel for
			for (int i = 0; i < this->pos.size(); ++i)
			{
				if (this->age[i] > 0.0f)
				{
					// Add forces.
					//acc[i] += this->gravity * dt;
					//this->acc[i] -= (this->vel[i] * this->drag.get());

					// Apply physics.
					this->vel[i] += this->acc[i];
					this->pos[i] += this->vel[i] * dt;
					this->vel[i] *= (1.0f - dt);
					this->acc[i] = glm::vec3(0.0f);

					if (glm::length(this->pos[i]) > this->worldBounds)
					{
						// Mark dead if out of bounds.
						this->age[i] = 0.0f;
					}
					else
					{
						// Get older.
						this->age[i] -= dt;
					}

					if (this->age[i] <= 0.0f)
					{
						this->zombies.push_back(i);
					}
				}

				this->links[i] = 0;
			}

			// Update the vbo.
			if (this->pos.size() <= this->vbo.getNumVertices())
			{
				this->vbo.updateVertexData(this->pos.data(), this->pos.size());
				this->vbo.updateAttributeData(ExtraAttribute::Age, this->age.data(), this->age.size());
			}
			else
			{
				this->vbo.setVertexData(this->pos.data(), this->pos.size(), GL_DYNAMIC_DRAW);
				this->vbo.setAttributeData(ExtraAttribute::Age, this->age.data(), 1, this->age.size(), GL_DYNAMIC_DRAW);
			}

			// Go through all particles and add lines to any that are close together.
			const auto minDistSq = (this->worldBounds * this->minDistance) * (this->worldBounds * this->minDistance);
			const auto maxDistSq = (this->worldBounds * this->maxDistance) * (this->worldBounds * this->maxDistance);
			std::vector<ofIndexType> indices;
			//auto & verts = this->vboMesh.getVertices();
//#pragma omp parallel for collapse(2)
			for (int i = 0; i < this->pos.size(); ++i)
			{
				if (this->age[i] > 0.0f && this->links[i] < this->maxLinks)
				{
					for (int j = i; j < this->pos.size(); ++j)
					{
						if (this->age[j] > 0.0f && this->links[j] < this->maxLinks)
						{
							const auto distSq = glm::distance2(this->pos[i], this->pos[j]);
							if (minDistSq <= distSq && distSq <= maxDistSq)
							{
								indices.push_back(i);
								indices.push_back(j);

								++this->links[i];
								++this->links[j];
							}
						}
					}
				}
			}

			this->vbo.setIndexData(indices.data(), indices.size(), GL_STATIC_DRAW);
		}

		//--------------------------------------------------------------
		void Bursts::draw(float alpha)
		{
			ofPushStyle();
			{
				ofEnableAlphaBlending();
				ofSetColor(this->color.get(), alpha * 255);

				this->shader.begin();
				{
					this->shader.setUniform1f("uMaxAge", this->maxAge);

					this->vbo.drawElements(GL_LINES, this->vbo.getNumIndices());
				}
				this->shader.end();
			}
			ofPopStyle();
		}
	}
}
