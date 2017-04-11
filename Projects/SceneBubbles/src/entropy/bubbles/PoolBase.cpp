#include "PoolBase.h"

namespace entropy
{
	namespace bubbles
	{
		//--------------------------------------------------------------
		PoolBase::PoolBase()
			: resetSimulation(false)
		{}

		//--------------------------------------------------------------
		void PoolBase::init()
		{
			this->resize();
			this->resetSimulation = true;
		}

		//--------------------------------------------------------------
		void PoolBase::reset()
		{
			this->frameCount = 0;

			this->prevIdx = 0;
			this->currIdx = 1;
			this->tempIdx = 2;

			this->resetSimulation = false;
		}

		//--------------------------------------------------------------
		void PoolBase::update(double dt)
		{
			if (this->resetSimulation)
			{
				this->reset();
			}

			if (this->runSimulation && (this->drawBack || this->drawFront))
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

		//--------------------------------------------------------------
		void PoolBase::gui(ofxPreset::Gui::Settings & settings)
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

				ofxPreset::Gui::EndTree(settings);
			}
		}

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
