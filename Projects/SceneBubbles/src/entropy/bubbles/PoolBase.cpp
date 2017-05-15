#include "PoolBase.h"

namespace entropy
{
	namespace bubbles
	{
		//--------------------------------------------------------------
		PoolBase::PoolBase()
			: needsReset(false)
		{}

		//--------------------------------------------------------------
		void PoolBase::init()
		{
			this->resize();
			this->needsReset = true;

			this->parameterListeners.push_back(this->resetSimulation.newListener([this](void)
			{
				this->needsReset = true;
			}));
		}

		//--------------------------------------------------------------
		void PoolBase::reset()
		{
			this->frameCount = 0;

			this->prevIdx = 0;
			this->currIdx = 1;
			this->tempIdx = 2;

			this->needsReset = false;
		}

		//--------------------------------------------------------------
		void PoolBase::update(double dt)
		{
			if (this->needsReset)
			{
				this->reset();
			}

			if (this->runSimulation && this->drawEnabled)
			{
				++this->frameCount;
				
				if (this->rippleRate == 1)
				{
					// Compute a new frame every frame.
					this->computeFrame();
					this->setDrawTextureIndex(this->currIdx);
				}
				else
				{
					int frame = (this->frameCount % this->rippleRate);
					if (frame == 1)
					{
						// Compute a new target frame for the next cycle.
						this->computeFrame();
					}
					if (frame == 0)
					{
						// End of the cycle, draw the previous computed frame.
						this->setDrawTextureIndex(this->currIdx);
					}
					else
					{
						// Mix previous and next frames during the cycle.
						float pct = frame / static_cast<float>(this->rippleRate.get());
						this->mixFrames(pct);
						this->setDrawTextureIndex(this->tempIdx);
					}
				}
			}
		}

		//--------------------------------------------------------------
		void PoolBase::computeFrame()
		{
			std::swap(this->currIdx, this->prevIdx);

			if (this->dropping && (ofGetFrameNum() % this->dropRate) == 0)
			{
				this->addDrop();
			}

			this->stepRipple();
			this->copyResult();
		}

		/*
		//--------------------------------------------------------------
		void PoolBase::gui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginTree(this->parameters, settings))
			{
				ofxImGui::AddParameter(this->runSimulation);
				ImGui::SameLine();
				if (ImGui::Button("Reset Simulation"))
				{
					this->resetSimulation = true;
				}

				ofxImGui::AddParameter(this->drawBack);
				ImGui::SameLine();
				ofxImGui::AddParameter(this->drawFront);

				ofxImGui::AddParameter(this->alpha);

				ofxImGui::AddParameter(this->dropColor1);
				ofxImGui::AddParameter(this->dropColor2);
				ofxImGui::AddParameter(this->dropping);
				ofxImGui::AddParameter(this->dropRate);

				ofxImGui::AddParameter(this->rippleRate);

				ofxImGui::AddParameter(this->damping);
				ofxImGui::AddParameter(this->radius);
				ofxImGui::AddParameter(this->ringSize);

				ofxImGui::EndTree(settings);
			}
		}
		*/

		//--------------------------------------------------------------
		void PoolBase::setDimensions(int size)
		{
			this->setDimensions(glm::vec3(size));
		}

		//--------------------------------------------------------------
		void PoolBase::setDimensions(const glm::vec2 & dimensions)
		{
			this->setDimensions(glm::vec3(dimensions, 0.0f));
		}

		//--------------------------------------------------------------
		void PoolBase::setDimensions(const glm::vec3 & dimensions)
		{
			this->dimensions = dimensions;
			this->resize();
		}

		//--------------------------------------------------------------
		const glm::vec3 & PoolBase::getDimensions() const
		{
			return this->dimensions;
		}
	}
}
