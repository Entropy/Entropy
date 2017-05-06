#include "Shape.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace geom
	{
		//--------------------------------------------------------------
		Shape::Shape()
			: meshDirty(true)
			, colorDirty(true)
		{
			//this->paramListeners.push_back(this->blendMode.newListener([this](int & mode)
			//{
			//	if (mode > OF_BLENDMODE_DISABLED)
			//	{
			//		this->depthTest = false;
			//	}
			//}));
			//this->paramListeners.push_back(this->depthTest.newListener([this](bool & enabled)
			//{
			//	if (enabled)
			//	{
			//		this->blendMode = OF_BLENDMODE_DISABLED;
			//	}
			//}));
			this->paramListeners.push_back(this->color.newListener([this](ofFloatColor &)
			{
				this->colorDirty = true;
			}));
			this->paramListeners.push_back(this->alpha.newListener([this](float &)
			{
				this->colorDirty = true;
			}));
		}

		//--------------------------------------------------------------
		Shape::~Shape()
		{
			this->clear();
			this->paramListeners.clear();
		}

		//--------------------------------------------------------------
		void Shape::clear()
		{
			this->mesh.clear();
		}

		//--------------------------------------------------------------
		void Shape::begin()
		{
			ofPushStyle();

			ofEnableBlendMode(static_cast<ofBlendMode>(this->blendMode.get()));
			this->depthTest ? ofEnableDepthTest() : ofDisableDepthTest();

			ofSetColor(this->color.get());

			const auto cullMode = static_cast<CullMode>(this->cullFace.get());
			if (cullMode != CullMode::Disabled)
			{
				glEnable(GL_CULL_FACE);
				if (cullMode == CullMode::Back)
				{
					glCullFace(GL_BACK);
				}
				else
				{
					glCullFace(GL_FRONT);
				}
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
		}

		//--------------------------------------------------------------
		void Shape::end()
		{
			const auto cullMode = static_cast<CullMode>(this->cullFace.get());
			if (cullMode != CullMode::Disabled)
			{
				glDisable(GL_CULL_FACE);
			}
	
			ofPopStyle();
		}

		//--------------------------------------------------------------
		void Shape::draw()
		{
			if (!this->enabled) return;

			this->begin();
			{
				this->getMesh().draw();
			}
			this->end();
		}

		//--------------------------------------------------------------
		const ofVboMesh & Shape::getMesh()
		{
			if (this->enabled && (this->meshDirty || this->colorDirty))
			{
				if (this->meshDirty)
				{
					this->rebuildMesh();
					this->colorDirty = true;
				}

				if (this->colorDirty)
				{
					this->mesh.getColors().resize(this->mesh.getVertices().size());
					ofFloatColor colorWithAlpha = this->color;
					colorWithAlpha.a = this->alpha;
					for (auto & c : this->mesh.getColors())
					{
						c = colorWithAlpha;
					}
					this->colorDirty = false;
				}
			}

			return this->mesh;
		}

		//--------------------------------------------------------------
		void Shape::addEdge(const glm::vec3 & center, const glm::vec3 & dimensions, int faces)
		{
			static const auto resX = 2;
			static const auto resY = 2;
			static const auto resZ = 2;

			const auto halfWidth = dimensions.x * .5f;
			const auto halfHeight = dimensions.y * .5f;
			const auto halfDepth = dimensions.z * .5f;

			glm::vec3 vertex;
			glm::vec2 texCoord;
			glm::vec3 normal;
			std::size_t vertOffset = this->mesh.getNumVertices();

			if ((faces & Face::Front) == Face::Front)
			{
				// Front Face.
				normal = { 0.0f, 0.0f, 1.0f };

				for (float y = 0; y < resY; ++y)
				{
					for (float x = 0; x < resX; ++x)
					{
						texCoord.x = x / (resX - 1.0f);
						texCoord.y = y / (resY - 1.0f);

						vertex.x = center.x + texCoord.x * dimensions.x - halfWidth;
						vertex.y = center.y + texCoord.y * dimensions.y - halfHeight;
						vertex.z = center.z + halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resY - 1; ++y) 
				{
					for (int x = 0; x < resX - 1; ++x) 
					{
						this->mesh.addIndex((y + 0) * resX + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);
					}
				}

				vertOffset = this->mesh.getNumVertices();
			}

			if ((faces & Face::Right) == Face::Right)
			{
				normal = { 1.0f, 0.0f, 0.0f };

				for (float y = 0; y < resY; ++y) 
				{
					for (float x = 0; x < resZ; ++x) 
					{
						texCoord.x = x / (resZ - 1.0f);
						texCoord.y = y / (resY - 1.0f);

						vertex.x = center.x + halfWidth;
						vertex.y = center.y + texCoord.y * dimensions.y - halfHeight;
						vertex.z = center.z + texCoord.x * -dimensions.z + halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resY - 1; ++y) 
				{
					for (int x = 0; x < resZ - 1; ++x) 
					{
						this->mesh.addIndex((y + 0) * resZ + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 0) + vertOffset);
					}
				}

				vertOffset = this->mesh.getNumVertices();
			}

			if ((faces & Face::Left) == Face::Left)
			{
				// Left Side Face.
				normal = { -1.0f, 0.0f, 0.0f };

				for (float y = 0; y < resY; ++y) 
				{
					for (float x = 0; x < resZ; ++x)
					{
						texCoord.x = x / (resZ - 1.0f);
						texCoord.y = y / (resY - 1.0f);

						vertex.x = center.x - halfWidth;
						vertex.y = center.y + texCoord.y * dimensions.y - halfHeight;
						vertex.z = center.z + texCoord.x * dimensions.z - halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resY - 1; ++y) 
				{
					for (int x = 0; x < resZ - 1; ++x) 
					{
						this->mesh.addIndex((y + 0) * resZ + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resZ + (x + 0) + vertOffset);
					}
				}

				vertOffset = this->mesh.getNumVertices();
			}

			if ((faces & Face::Back) == Face::Back)
			{
				// Back Face.
				normal = { 0.0f, 0.0f, -1.0f };

				for (float y = 0; y < resY; ++y) 
				{
					for (float x = 0; x < resX; ++x) 
					{
						texCoord.x = x / (resX - 1.0f);
						texCoord.y = y / (resY - 1.0f);

						vertex.x = center.x + texCoord.x * -dimensions.x + halfWidth;
						vertex.y = center.y + texCoord.y * dimensions.y - halfHeight;
						vertex.z = center.z - halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resY - 1; ++y)
				{
					for (int x = 0; x < resX - 1; ++x)
					{
						this->mesh.addIndex((y + 0) * resX + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);
					}
				}

				vertOffset = this->mesh.getNumVertices();
			}

			if ((faces & Face::Bottom) == Face::Bottom)
			{
				// Top Face.
				normal = { 0.0f, -1.0f, 0.0f };

				for (float y = 0; y < resZ; ++y)
				{
					for (float x = 0; x < resX; ++x)
					{
						texCoord.x = x / (resX - 1.0f);
						texCoord.y = y / (resZ - 1.0f);

						vertex.x = center.x + texCoord.x * dimensions.x - halfWidth;
						vertex.y = center.y - halfHeight;
						vertex.z = center.z + texCoord.y * dimensions.z - halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resZ - 1; ++y) 
				{
					for (int x = 0; x < resX - 1; ++x)
					{
						this->mesh.addIndex((y + 0) * resX + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);
					}
				}

				vertOffset = this->mesh.getNumVertices();
			}

			if ((faces & Face::Top) == Face::Top)
			{
				// Bottom Face.
				normal = { 0.0f, 1.0f, 0.0f };

				for (float y = 0; y < resZ; ++y) 
				{
					for (float x = 0; x < resX; ++x)
					{
						texCoord.x = x / (resX - 1.0f);
						texCoord.y = y / (resZ - 1.0f);

						vertex.x = center.x + texCoord.x * dimensions.x - halfWidth;
						vertex.y = center.y + halfHeight;
						vertex.z = center.z + texCoord.y * -dimensions.z + halfDepth;

						this->mesh.addVertex(vertex);
						this->mesh.addTexCoord(texCoord);
						this->mesh.addNormal(normal);
					}
				}

				for (int y = 0; y < resZ - 1; ++y)
				{
					for (int x = 0; x < resX - 1; ++x) 
					{
						this->mesh.addIndex((y + 0) * resX + (x + 0) + vertOffset);
						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);

						this->mesh.addIndex((y + 0) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 1) + vertOffset);
						this->mesh.addIndex((y + 1) * resX + (x + 0) + vertOffset);
					}
				}
			}
		}
	}
}