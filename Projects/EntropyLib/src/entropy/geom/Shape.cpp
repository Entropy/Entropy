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
			this->paramListeners.push_back(this->alphaBlend.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->depthTest = false;
				}
			}));
			this->paramListeners.push_back(this->depthTest.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->alphaBlend = false;
				}
			}));
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
		void Shape::draw()
		{
			if (!this->enabled) return;

			ofPushStyle();
			{
				this->alphaBlend ? ofEnableAlphaBlending() : ofDisableAlphaBlending();
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

				this->getMesh().draw();

				if (cullMode != CullMode::Disabled)
				{
					glDisable(GL_CULL_FACE);
				}
			}
			ofPopStyle();
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
	}
}