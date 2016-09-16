#include "Stripes.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace geom
	{
		//--------------------------------------------------------------
		Stripes::Stripes()
			: Shape()
		{
			// Update parameter group.
			this->parameters.setName("Stripes");
			this->parameters.add(this->lineWidth, this->lineHeight, this->spaceWidth, this->count, this->zPosition);

			// Add parameter listeners.
			this->paramListeners.push_back(this->lineWidth.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->lineHeight.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->spaceWidth.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->count.newListener([this](int &)
			{
				this->meshDirty = true;
			}));
		}

		//--------------------------------------------------------------
		Stripes::~Stripes()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Stripes::draw()
		{
			ofPushMatrix();
			{
				ofTranslate(0.0f, 0.0f, this->zPosition);

				Shape::draw();
			}
			ofPopMatrix();
		}

		//--------------------------------------------------------------
		void Stripes::rebuildMesh()
		{
			this->clear();
			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			const auto totalWidth = (this->lineWidth * this->count) + (this->spaceWidth * (this->count - 1));
			const auto edgeOffset = this->lineWidth + this->spaceWidth;
			const auto dimensions = glm::vec3(this->lineWidth, this->lineHeight, 1.0f);

			auto center = glm::vec3(0.0f - totalWidth * 0.5f, 0.0f, 0.0f);
			for (int i = 0; i < this->count; ++i)
			{
				this->addEdge(center, dimensions, Face::Front);
				center.x += edgeOffset;
			}

			this->meshDirty = false;
		}
	}
}
