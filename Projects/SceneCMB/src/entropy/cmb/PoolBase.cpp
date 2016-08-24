#include "PoolBase.h"

namespace entropy
{
	namespace cmb
	{
		//--------------------------------------------------------------
		PoolBase::PoolBase()
			: restartSimulation(false)
		{}

		//--------------------------------------------------------------
		void PoolBase::setup()
		{
			this->prevIdx = 0;
			this->currIdx = 1;
			this->tempIdx = 2;

			this->restartSimulation = true;
		}

		//--------------------------------------------------------------
		void PoolBase::update()
		{
			if (this->restartSimulation)
			{
				this->setup();
				this->restartSimulation = false;
			}

			auto & parameters = this->getParameters();
			if (parameters.base.dropping && (ofGetFrameNum() % parameters.base.dropRate) == 0)
			{
				this->addDrop();
			}

			if ((ofGetFrameNum() % parameters.base.rippleRate) == 0)
			{
				this->stepRipple();
				this->copyResult();

				std::swap(this->currIdx, this->prevIdx);
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
			setDimensions(glm::vec3(dimensions, 0.0f));
		}

		//--------------------------------------------------------------
		void PoolBase::setDimensions(const glm::vec3 & dimensions)
		{
			this->dimensions = dimensions;
			this->restartSimulation = true;
		}
	}
}
