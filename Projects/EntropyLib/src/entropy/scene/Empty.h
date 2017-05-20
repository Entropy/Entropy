#pragma once

#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Empty
			: public Base
		{
		public:
			std::string getName() const override
			{
				return "entropy::scene::Empty";
			}

		protected:
			ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			ofParameterGroup parameters
			{
				"Empty"
			};
		};
	}
}