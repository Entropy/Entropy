#include "Sphere.h"

namespace entropy
{
	namespace geom
	{
		//--------------------------------------------------------------
		Sphere::Sphere()
			: meshDirty(true)
		{
			this->paramListeners.push_back(this->radius.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->resolution.newListener([this](int &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->arcLength.newListener([this](float &)
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
		void Sphere::clear()
		{
			this->mesh.clear();
		}

		//--------------------------------------------------------------
		bool Sphere::update()
		{
			if (this->enabled)
			{
				if (this->meshDirty)
				{
					this->rebuildMesh();
					return true;
				}
			}
			return false;
		}

		//--------------------------------------------------------------
		void Sphere::draw() const
		{
			if (!this->enabled) return;

			ofPushStyle();
			{
				ofEnableAlphaBlending();
				ofSetColor(ofColor::white);

				this->mesh.draw();
			}
			ofPopStyle();
		}

		//--------------------------------------------------------------
		void Sphere::rebuildMesh()
		{
			this->clear();
			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			const auto arcResolution = static_cast<int>(ofMap(this->arcLength, 0.0f, 1.0f, 0, this->resolution));
			const auto doubleResolution = this->resolution * 2.0f;

			const auto polarInc = PI / (this->resolution);
			const auto azimInc = TWO_PI / (doubleResolution);

			glm::vec3 vertex;
			glm::vec2 texcoord;

			for (float i = 0; i < arcResolution + 1; ++i)
			{
				float tr = sin(PI - i * polarInc);
				float ny = cos(PI - i * polarInc);

				texcoord.y = i / this->resolution;

				for (float j = 0; j <= doubleResolution; ++j) 
				{
					float nx = tr * sin(j * azimInc);
					float nz = tr * cos(j * azimInc);

					texcoord.x = j / (doubleResolution);

					vertex = glm::vec3(nx, ny, nz);
					mesh.addNormal(vertex);
					vertex *= radius;
					mesh.addVertex(vertex);
					mesh.addTexCoord(texcoord);
				}
			}

			int nr = doubleResolution + 1;
			ofIndexType i0, i1, i2;
			for (float y = 0; y < arcResolution; ++y)
			{
				for (float x = 0; x < doubleResolution; ++x)
				{
					// first tri //
					if (y > 0) 
					{
						i0 = (y + 0) * nr + (x + 0);
						i1 = (y + 0) * nr + (x + 1);
						i2 = (y + 1) * nr + (x + 0);

						mesh.addIndex(i0);
						mesh.addIndex(i1);
						mesh.addIndex(i2);
					}

					if (y < arcResolution - 1) 
					{
						// second tri //
						i0 = (y + 0) * nr + (x + 1);
						i1 = (y + 1) * nr + (x + 1);
						i2 = (y + 1) * nr + (x + 0);

						mesh.addIndex(i0);
						mesh.addIndex(i1);
						mesh.addIndex(i2);

					}
				}
			}

			this->meshDirty = false;
		}

		//--------------------------------------------------------------
		const ofVboMesh & Sphere::getMesh()
		{
			return this->mesh;
		}
	}
}