#include "PoolGL3D.h"

#ifdef COMPUTE_GL_3D

#include "entropy/Helpers.h"

namespace entropy
{
	namespace cmb
	{
		//--------------------------------------------------------------
		PoolGL3D::PoolGL3D()
			: PoolBase()
		{}

		//--------------------------------------------------------------
		void PoolGL3D::setup()
		{
			PoolBase::setup();

			// Allocate the textures and buffers.
			for (int i = 0; i < 3; ++i) {
				this->textures[i].allocate(this->dimensions.x, this->dimensions.y, this->dimensions.z, GL_RGBA16F);

				this->fbos[i].allocate();
				this->fbos[i].attachTexture(this->textures[i], 0);
				this->fbos[i].begin();
				{
					ofClear(0, 0);
				}
				this->fbos[i].end();
				this->fbos[i].checkStatus();
			}

			// Build a mesh to render a quad.
			const auto origin = glm::vec3(0.0f, GetCanvasHeight() - this->dimensions.y, 0.0f);

			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);
			this->mesh.addVertex(origin + glm::vec3(0.0f, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(0.0, this->dimensions.y, 0.0f));

			this->mesh.addTexCoord(glm::vec2(0.0f, 0.0f));
			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, 0.0f));
			this->mesh.addTexCoord(glm::vec2(0.0f, this->dimensions.y));

			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(0.0f, this->dimensions.y, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, this->dimensions.y, 0.0f));

			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, 0.0f));
			this->mesh.addTexCoord(glm::vec2(0.0f, this->dimensions.y));
			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, this->dimensions.y));

			// Load the shaders.
			this->dropShader.load("shaders/passthru.vert", "shaders/drop3D.frag", "shaders/layer.geom");
			this->rippleShader.load("shaders/passthru.vert", "shaders/ripple3D.frag", "shaders/layer.geom");
			this->copyShader.load("shaders/passthru.vert", "shaders/copy3D.frag", "shaders/layer.geom");
		}

		//--------------------------------------------------------------
		void PoolGL3D::addDrop()
		{
			this->fbos[this->prevIdx].begin();
			{
				ofEnableAlphaBlending();
				ofSetColor(this->parameters.base.dropColor.get());

				const auto burstPos = glm::vec3(ofRandom(this->dimensions.x), ofRandom(this->dimensions.y), ofRandom(this->dimensions.z));
				const auto burstThickness = 1.0f;

				this->dropShader.begin();
				{
					this->dropShader.setUniform3f("uBurst.pos", burstPos);
					this->dropShader.setUniform1f("uBurst.radius", this->parameters.base.radius);
					this->dropShader.setUniform1f("uBurst.thickness", burstThickness);
					//this->dropShader.printActiveUniforms();

					int minLayer = static_cast<int>(max(0.0f, burstPos.z - this->parameters.base.radius - burstThickness));
					int maxLayer = static_cast<int>(min(this->dimensions.z - 1, burstPos.z + this->parameters.base.radius + burstThickness));
					for (int i = minLayer; i <= maxLayer; ++i) {
					//for (int i = 0; i < this->dimensions.z; ++i) {
						this->dropShader.setUniform1i("uLayer", i);
						this->mesh.draw();
					}
				}
				this->dropShader.end();
			}
			this->fbos[this->prevIdx].end();
		}

		//--------------------------------------------------------------
		void PoolGL3D::stepRipple()
		{
			this->fbos[this->tempIdx].begin();
			{
				ofDisableAlphaBlending();

				this->rippleShader.begin();
				{
					this->rippleShader.setUniform1f("uDamping", this->parameters.base.damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
					this->rippleShader.setUniformTexture("uPrevBuffer", this->textures[this->prevIdx].texData.textureTarget, this->textures[this->prevIdx].texData.textureID, 1);
					this->rippleShader.setUniformTexture("uCurrBuffer", this->textures[this->currIdx].texData.textureTarget, this->textures[this->currIdx].texData.textureID, 2);
					this->rippleShader.setUniform3f("uDims", this->dimensions);

					for (int i = 0; i < this->dimensions.z; ++i)
					{
						this->rippleShader.setUniform1i("uLayer", i);
						this->mesh.draw();
					}
				}
				this->rippleShader.end();
			}
			this->fbos[this->tempIdx].end();
		}

		//--------------------------------------------------------------
		void PoolGL3D::copyResult()
		{
			this->fbos[this->currIdx].begin();
			{
				ofDisableAlphaBlending();

				this->copyShader.begin();
				{
					this->copyShader.setUniformTexture("uCopyBuffer", this->textures[this->tempIdx].texData.textureTarget, this->textures[this->tempIdx].texData.textureID, 1);
					this->copyShader.setUniform3f("uDims", this->dimensions);
					{
						for (int i = 0; i < this->dimensions.z; ++i)
						{
							this->copyShader.setUniform1i("uLayer", i);
							this->mesh.draw();
						}
					}
				}
				this->copyShader.end();
			}
			this->fbos[this->currIdx].end();

			this->volumetrics.setup(&this->textures[this->currIdx], ofVec3f(1, 1, 1));
		}

		//--------------------------------------------------------------
		void PoolGL3D::draw()
		{
			this->volumetrics.setRenderSettings(1.0, 1.0, 1.0, 0.1);
			this->volumetrics.setVolumeTextureFilterMode(this->parameters.volumetrics.filterMode);
			this->volumetrics.drawVolume(0.0f, 0.0f, 0.0f, this->parameters.volumetrics.volumeSize, 0);

			//this->mesh.draw();
		}
	}
}

#endif COMPUTE_GL_3D