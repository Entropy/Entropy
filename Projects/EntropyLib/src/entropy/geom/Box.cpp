#include "Box.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace geom
	{
		//--------------------------------------------------------------
		Box::Box()
			: Shape()
		{
			// Update parameter group.
			this->parameters.setName("Box");
			this->parameters.add(this->size, this->edgeRatio, this->subdivisions);

			// Add parameter listeners.
			this->paramListeners.push_back(this->size.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->edgeRatio.newListener([this](float &)
			{
				this->meshDirty = true;
			}));
			this->paramListeners.push_back(this->subdivisions.newListener([this](int &)
			{
				this->meshDirty = true;
			}));
		}

		//--------------------------------------------------------------
		Box::~Box()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Box::draw(render::WireframeFillRenderer & renderer, ofCamera & camera)
		{
			if (!this->enabled) return;

			this->begin();
			{
				renderer.drawElements(this->getMesh().getVbo(), 0, this->getMesh().getNumIndices(), camera);
			}
			this->end();
		}

		//--------------------------------------------------------------
		void Box::rebuildMesh()
		{
			this->clear();
			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			const auto edgeOffset = this->size / this->subdivisions;
			const auto edgeWidth = this->edgeRatio * this->size;
			glm::vec3 center;
			glm::vec3 dimensions;

			// Front.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f, 0.0f - size * 0.5f, 0.0f + size * 0.5f);
					dimensions = glm::vec3(size + edgeWidth, edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Front);
						center.y += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f, 0.0f + size * 0.5f);
					dimensions = glm::vec3(edgeWidth, size + edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Front);
						center.x += edgeOffset;
					}
				}
			}

			// Back.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f, 0.0f - size * 0.5f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(size + edgeWidth, edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Back);
						center.y += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(edgeWidth, size + edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Back);
						center.x += edgeOffset;
					}
				}
			}

			// Left.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f - size * 0.5f, 0.0f);
					dimensions = glm::vec3(edgeWidth, edgeWidth, size + edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Left);
						center.y += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(edgeWidth, size + edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Left);
						center.z += edgeOffset;
					}
				}
			}

			// Right.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f + size * 0.5f, 0.0f - size * 0.5f, 0.0f);
					dimensions = glm::vec3(edgeWidth, edgeWidth, size + edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Right);
						center.y += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f + size * 0.5f, 0.0f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(edgeWidth, size + edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Right);
						center.z += edgeOffset;
					}
				}
			}

			// Top.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f, 0.0f + size * 0.5f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(size + edgeWidth, edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Top);
						center.z += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f + size * 0.5f, 0.0f);
					dimensions = glm::vec3(edgeWidth, edgeWidth, size + edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Top);
						center.x += edgeOffset;
					}
				}
			}

			// Bottom.
			{
				// Horizontal.
				{
					center = glm::vec3(0.0f, 0.0f - size * 0.5f, 0.0f - size * 0.5f);
					dimensions = glm::vec3(size + edgeWidth, edgeWidth, edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Bottom);
						center.z += edgeOffset;
					}
				}
				// Vertical.
				{
					center = glm::vec3(0.0f - size * 0.5f, 0.0f - size * 0.5f, 0.0f);
					dimensions = glm::vec3(edgeWidth, edgeWidth, size + edgeWidth);
					for (int i = 0; i <= subdivisions; ++i)
					{
						this->addEdge(center, dimensions, Face::Bottom);
						center.x += edgeOffset;
					}
				}
			}

			this->meshDirty = false;
		}
	}
}
