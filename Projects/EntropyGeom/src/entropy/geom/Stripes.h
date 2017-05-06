#pragma once

#include "Shape.h"

namespace entropy
{
	namespace geom
	{
		class Stripes
			: public Shape
		{
		public:
			Stripes();
			~Stripes();

			void begin() override;
			void end() override;

			ofParameter<float> lineWidth{ "Line Width", 1.0f, 0.0f, 1000.0f };
			ofParameter<float> lineHeight{ "Line Height", 1.0f, 0.0f, 1000.0f };
			ofParameter<int> lineCount{ "Line Count", 1, 1, 100 };
			ofParameter<float> spaceWidth{ "Space Width", 1.0f, 0.0f, 1000.0f };
			ofParameter<float> zPosition{ "Z Position", 0.0f, -1000.0f, 1000.0f };

		protected:
			void rebuildMesh() override;
		};
	}
}