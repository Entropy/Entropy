#pragma once

#include "Base.h"

namespace entropy
{
	namespace media
	{
		class Link
		{
		public:
			Link(size_t idxA, size_t idxB, bool direction, std::vector<std::shared_ptr<media::Base>> & medias);
			~Link();

			size_t getIdxA() const;
			size_t getIdxB() const;
			bool getDirection() const;

			void setLabel(const std::string & label);
			const std::string & getLabel() const;

		protected:
			std::shared_ptr<media::Base> mediaA;
			std::shared_ptr<media::Base> mediaB;
			std::vector<ofEventListener> listeners;

			size_t idxA;
			size_t idxB;
			bool direction;

			std::string label;
		};
	}
}