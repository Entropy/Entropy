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
		void PoolBase::setup()
		{
			this->resetSimulation = true;
		}

		//--------------------------------------------------------------
		void PoolBase::reset()
		{
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
				if (this->dropping && (ofGetFrameNum() % this->dropRate) == 0)
				{
					this->addDrop();
				}

				if ((ofGetFrameNum() % this->rippleRate) == 0)
				{
					this->stepRipple();
					this->copyResult();

					std::swap(this->currIdx, this->prevIdx);
				}
			}
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
			this->setup();
		}

		//--------------------------------------------------------------
		const glm::vec3 & PoolBase::getDimensions() const
		{
			return this->dimensions;
		}
	}
}
