#include "PoolGL3D.h"

#ifdef COMPUTE_GL_3D

#include "entropy/Helpers.h"

namespace entropy
{
	namespace bubbles
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
			for (int i = 0; i < 3; ++i) 
			{
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

			this->copyBuffer.allocate(this->dimensions.x * this->dimensions.y * this->dimensions.z * 2 * 4, GL_STATIC_DRAW);

			this->volumetrics.setup(&this->textures[0], glm::vec3(1.0f));

			// Build a mesh to render a quad.
			const auto origin = glm::vec3(0.0f, GetCanvasHeight(render::Layout::Back) - this->dimensions.y, 0.0f);

			this->mesh.clear();
			this->mesh.setMode(OF_PRIMITIVE_TRIANGLES);

			this->mesh.addVertex(origin + glm::vec3(0.0f, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(0.0, this->dimensions.y, 0.0f));

			this->mesh.addTexCoord(glm::vec2(0.0f, 0.0f));
			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, 0.0f));
			this->mesh.addTexCoord(glm::vec2(0.0f, this->dimensions.y));

			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, 0.0f, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(this->dimensions.x, this->dimensions.y, 0.0f));
			this->mesh.addVertex(origin + glm::vec3(0.0f, this->dimensions.y, 0.0f));

			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, 0.0f));
			this->mesh.addTexCoord(glm::vec2(this->dimensions.x, this->dimensions.y));
			this->mesh.addTexCoord(glm::vec2(0.0f, this->dimensions.y));

			// Load the shaders.
			auto shaderSettings = ofShader::Settings();
			shaderSettings.bindDefaults = true;
			shaderSettings.intDefines["USE_TEX_ARRAY"] = USE_TEX_ARRAY;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/passthru.vert";
			shaderSettings.shaderFiles[GL_GEOMETRY_SHADER] = "shaders/layer.geom";

			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/drop3D.frag";
			this->dropShader.setup(shaderSettings);
			
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/ripple3D.frag";
			this->rippleShader.setup(shaderSettings);

			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/copy3D.frag";
			this->copyShader.setup(shaderSettings);

			// Init bursts.
			this->bursts.init();

			// Init parameters.
			this->parameters.setName("Pool GL 3D");
			this->parameters.add(filterMode, volumeSize);
			this->parameters.add(this->bursts.parameters);
		}

		//--------------------------------------------------------------
		void PoolGL3D::reset()
		{
			PoolBase::reset();

			// Clear the textures and buffers.
			for (int i = 0; i < 3; ++i)
			{
				this->textures[i].clearData();
			}

			this->bursts.reset();
		}

		//--------------------------------------------------------------
		void PoolGL3D::update(double dt)
		{
			PoolBase::update(dt);

			if (this->bursts.enabled)
			{
				this->bursts.update(dt);
			}
		}

		//--------------------------------------------------------------
		void PoolGL3D::addDrop()
		{
			const auto burstPos = glm::vec3(ofRandom(this->dimensions.x), ofRandom(this->dimensions.y), ofRandom(this->dimensions.z));
			static const auto burstThickness = 1.0f;
			
			this->fbos[this->prevIdx].begin();
			{
				ofEnableAlphaBlending();
				ofSetColor((ofRandomuf() < 0.5 ? this->dropColor1.get() : this->dropColor2.get()));

				this->dropShader.begin();
				{
					this->dropShader.setUniform3f("uBurst.pos", burstPos);
					this->dropShader.setUniform1f("uBurst.radius", this->radius);
					this->dropShader.setUniform1f("uBurst.thickness", burstThickness);
					this->dropShader.setUniform3f("uDims", this->dimensions);
					
					int minLayer = static_cast<int>(std::max(0.0f, burstPos.z - this->radius - burstThickness));
					int maxLayer = static_cast<int>(std::min(this->dimensions.z - 1, burstPos.z + this->radius + burstThickness));
					for (int i = minLayer; i <= maxLayer; ++i) {
					//for (int i = 0; i < this->dimensions.z; ++i) {
						this->dropShader.setUniform1i("uLayer", i);
						this->mesh.draw();
					}
				}
				this->dropShader.end();
			}
			this->fbos[this->prevIdx].end();

			if (this->bursts.enabled)
			{
				static const auto halfVec = glm::vec3(0.5f);
				auto worldPos = burstPos / this->dimensions;
				worldPos.y = 1.0f - worldPos.y;
				worldPos = (worldPos - halfVec) * this->volumeSize.get();
				this->bursts.addDrop(worldPos, this->radius);
			}
		}

		//--------------------------------------------------------------
		void PoolGL3D::stepRipple()
		{
			this->fbos[this->tempIdx].begin();
			{
				ofDisableAlphaBlending();
				
				this->rippleShader.begin();
				{
					this->rippleShader.setUniform1f("uDamping", this->damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
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

			//this->textures[this->tempIdx].copyTo(this->copyBuffer);
			//this->textures[this->currIdx].loadData(this->copyBuffer, ofGetGLFormatFromInternal(this->textures[this->tempIdx].texData.glInternalFormat));

			this->volumetrics.updateTexture(&this->textures[this->currIdx], glm::vec3(1.0f));
			//this->volumetrics.updateTexture(&this->textures[this->prevIdx], glm::vec3(1.0f));
		}

		//--------------------------------------------------------------
		void PoolGL3D::draw()
		{
			ofPushStyle();
			{
				ofEnableAlphaBlending();
				ofSetColor(255, this->alpha * 255);

				this->volumetrics.setRenderSettings(1.0, 1.0, 1.0, 0.1);
				this->volumetrics.setVolumeTextureFilterMode(this->filterMode);
				this->volumetrics.drawVolume(0.0f, 0.0f, 0.0f, this->volumeSize, 0);
			}
			ofPopStyle();

			if (this->bursts.enabled)
			{
				this->bursts.draw(this->alpha);
			}
		}

		//--------------------------------------------------------------
		void PoolGL3D::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ofxPreset::Gui::BeginTree(this->parameters, settings))
			{
				ofxPreset::Gui::AddParameter(this->runSimulation);
				ImGui::SameLine();
				if (ImGui::Button("Reset Simulation"))
				{
					this->resetSimulation = true;
				}

				ofxPreset::Gui::AddParameter(this->drawBack);
				ImGui::SameLine();
				ofxPreset::Gui::AddParameter(this->drawFront);

				ofxPreset::Gui::AddParameter(this->alpha);

				ofxPreset::Gui::AddParameter(this->dropColor1);
				ofxPreset::Gui::AddParameter(this->dropColor2);
				ofxPreset::Gui::AddParameter(this->dropping);
				ofxPreset::Gui::AddParameter(this->dropRate);

				ofxPreset::Gui::AddParameter(this->rippleRate);

				ofxPreset::Gui::AddParameter(this->damping);
				ofxPreset::Gui::AddParameter(this->radius);
				ofxPreset::Gui::AddParameter(this->ringSize);

				static const vector<string> labels{ "Nearest", "Linear" };
				ofxPreset::Gui::AddRadio(this->filterMode, labels, 2);
				ofxPreset::Gui::AddParameter(this->volumeSize);

				if (ofxPreset::Gui::BeginTree(this->bursts.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->bursts.enabled);
					ofxPreset::Gui::AddParameter(this->bursts.resolution);
					ofxPreset::Gui::AddParameter(this->bursts.color);
					ofxPreset::Gui::AddParameter(this->bursts.maxAge);
					ofxPreset::Gui::AddParameter(this->bursts.forceMultiplier);
					ofxPreset::Gui::AddParameter(this->bursts.forceRandom);
					ofxPreset::Gui::AddParameter(this->bursts.worldBounds);
					ofxPreset::Gui::AddRange("Distance", this->bursts.minDistance, this->bursts.maxDistance);
					ofxPreset::Gui::AddParameter(this->bursts.maxLinks);

					ofxPreset::Gui::EndTree(settings);
				}

				ofxPreset::Gui::EndTree(settings);
			}
		}

		//--------------------------------------------------------------
		const ofxTexture & PoolGL3D::getTexture() const
		{
			return this->textures[this->prevIdx];
		}
	}
}

#endif // COMPUTE_GL_3D
