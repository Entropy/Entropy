#include "Link.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Link::Link(size_t idxA, size_t idxB, bool direction, std::vector<std::shared_ptr<media::Asset>> & medias)
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

			this->mediaA->setLinkedMedia(this->mediaB);
			this->mediaB->setLinkedMedia(this->mediaA);

			this->label = this->mediaA->getTypeName() + " " + ofToString(idxA) + " <=> " + this->mediaB->getTypeName() + " " + ofToString(idxB) + " " + (direction ? "[+]" : "[-]");
			
			this->listeners.push_back(this->mediaA->parameters.playback.fadeTwist.newListener([this](float & val)
			{
				this->mediaB->parameters.playback.fadeTwist = (this->direction ? val : (1.0f - val));
			}));
			this->listeners.push_back(this->mediaB->parameters.playback.fadeTwist.newListener([this](float & val)
			{
				this->mediaA->parameters.playback.fadeTwist = (this->direction ? val : (1.0f - val));
			}));

			// Set the first time.
			this->mediaB->parameters.playback.fadeTwist = (this->direction ? this->mediaA->parameters.playback.fadeTwist : (1.0f - this->mediaA->parameters.playback.fadeTwist));
		}
		
		//--------------------------------------------------------------
		Link::~Link()
		{
			if (this->mediaA != nullptr)
			{
				this->mediaA->clearLinkedMedia();
			}
			if (this->mediaB != nullptr)
			{
				this->mediaB->clearLinkedMedia();
			}
		}
		
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