#include "Sphere.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace geom
	{
		//--------------------------------------------------------------
		Sphere::Sphere()
			: Shape()
		{
			// Update parameter group.
			this->parameters.setName("Sphere");
			this->parameters.add(this->radius, this->resolution, this->arcHorz, this->arcVert);
			
			// Add parameter listeners.
			this->paramListeners.push_back(this->radius.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->resolution.newListener([this](int &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->arcHorz.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->arcVert.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
		}

		//--------------------------------------------------------------
		Sphere::~Sphere()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Sphere::begin()
		{
			ofPushMatrix();
			
			ofRotateXRad(this->orientation.get().x);
			ofRotateYRad(this->orientation.get().y);
			ofRotateZRad(this->orientation.get().z);

			Shape::begin();
		}

		//--------------------------------------------------------------
		void Sphere::end()
		{
			Shape::end();

			ofPopMatrix();
		}

		//--------------------------------------------------------------
		void Sphere::rebuildMesh()
		{
			this->clear();
			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			const int doubleResolution = this->resolution * 2;
			const int vertResolution = static_cast<int>(ofMap(this->arcVert, 0.0f, 1.0f, 0, this->resolution));
			const int horzResolution = static_cast<int>(ofMap(this->arcHorz, 0.0f, 1.0f, 0, doubleResolution));

			const float polarInc = glm::pi<float>() / this->resolution;
			const float azimInc = glm::two_pi<float>() / doubleResolution;

			glm::vec3 vertex;
			glm::vec2 texcoord;

			for (int i = 0; i < vertResolution + 1; ++i)
			{
				float tr = sin(glm::pi<float>() - i * polarInc);
				float ny = cos(glm::pi<float>() - i * polarInc);

				texcoord.y = i / static_cast<float>(this->resolution);

				for (int j = 0; j <= horzResolution; ++j)
				{
					float nx = tr * sin(j * azimInc);
					float nz = tr * cos(j * azimInc);

					texcoord.x = j / static_cast<float>(doubleResolution);

					vertex = glm::vec3(nx, ny, nz);
					this->mesh.addNormal(vertex);
					vertex *= radius;
					this->mesh.addVertex(vertex);
					this->mesh.addTexCoord(texcoord);
				}
			}

			int nr = horzResolution + 1;
			ofIndexType i0, i1, i2;
			for (int y = 0; y < vertResolution; ++y)
			{
				for (int x = 0; x < horzResolution; ++x)
				{
					// First triangle.
					if (y > 0 && (this->arcVert == 1.0f || y < vertResolution - 1)) 
					{
						i0 = (y + 0) * nr + (x + 0);
						i1 = (y + 0) * nr + (x + 1);
						i2 = (y + 1) * nr + (x + 0);

						this->mesh.addIndex(i0);
						this->mesh.addIndex(i1);
						this->mesh.addIndex(i2);
					}

					if (y < vertResolution - 1)
					{
						// Second triangle.
						i0 = (y + 0) * nr + (x + 1);
						i1 = (y + 1) * nr + (x + 1);
						i2 = (y + 1) * nr + (x + 0);

						this->mesh.addIndex(i0);
						this->mesh.addIndex(i1);
						this->mesh.addIndex(i2);
					}
				}
			}

			this->meshDirty = false;
		}
	}
}