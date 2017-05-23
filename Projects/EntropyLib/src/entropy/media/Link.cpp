#include "Link.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Link::Link(size_t idxA, size_t idxB, bool direction, std::vector<std::shared_ptr<media::Base>> & medias)
			: idxA(idxA)
			, idxB(idxB)
			, direction(direction)
			, label("")
		{
			if (this->idxA >= medias.size() || this->idxB >= medias.size())
			{
				ofLogError(__FUNCTION__) << "Indices " << this->idxA << " and/or " << this->idxB << " are out of range for size " << medias.size();
				return;
			}
			
			this->mediaA = medias[idxA];
			this->mediaB = medias[idxB];

			this->label = this->mediaA->getTypeName() + " " + ofToString(idxA) + " <=> " + this->mediaB->getTypeName() + " " + ofToString(idxB) + " " + (direction ? "[+]" : "[-]");
			
			auto & paramsA = this->mediaA->getParameters();
			this->listeners.push_back(paramsA.base.fade.newListener([this](float & val)
			{
				auto & paramsB = this->mediaB->getParameters();
				paramsB.base.fade = (this->direction ? val : (1.0f - val));
			}));
			auto & paramsB = this->mediaB->getParameters();
			this->listeners.push_back(paramsB.base.fade.newListener([this](float & val)
			{
				auto & paramsA = this->mediaA->getParameters();
				paramsA.base.fade = (this->direction ? val : (1.0f - val));
			}));

			// Set the first time.
			paramsB.base.fade = (this->direction ? paramsA.base.fade : (1.0f - paramsA.base.fade));
		}
		
		//--------------------------------------------------------------
		Link::~Link()
		{}
		
		//--------------------------------------------------------------
		size_t Link::getIdxA() const
		{
			return this->idxA;
		}
		
		//--------------------------------------------------------------
		size_t Link::getIdxB() const
		{
			return this->idxB;
		}

		//--------------------------------------------------------------
		bool Link::getDirection() const
		{
			return this->direction;
		}

		//--------------------------------------------------------------
		void Link::setLabel(const std::string & label)
		{
			this->label = label;
		}

		//--------------------------------------------------------------
		const std::string & Link::getLabel() const
		{
			return this->label;
		}
	}
}