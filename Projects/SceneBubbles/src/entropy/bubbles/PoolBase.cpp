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
		void PoolBase::update()
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
	}
}
