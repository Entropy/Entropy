#pragma once

#include "Asset.h"

namespace entropy
{
	namespace media
	{
		class Link
		{
		public:
			Link(size_t idxA, size_t idxB, bool direction, std::vector<std::shared_ptr<media::Asset>> & medias);
			~Link();

			size_t getIdxA() const;
			size_t getIdxB() const;
			bool getDirection() const;

			void setLabel(const std::string & label);
			const std::string & getLabel() const;

		protected:
			std::shared_ptr<media::Asset> mediaA;
			std::shared_ptr<media::Asset> mediaB;
			std::vector<ofEventListener> listeners;

			size_t idxA;
			size_t idxB;
			bool direction;

			std::string label;
		};
	}
}