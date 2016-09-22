#pragma once

#include "Shape.h"

namespace entropy
{
	namespace geom
	{
		class Sphere
			: public Shape
		{
		public:
			Sphere();
			~Sphere();

			void begin() override;
			void end() override;

			ofParameter<float> radius{ "Radius", 0.1f, 0.001f, 1.0f };
			ofParameter<int> resolution{ "Resolution", 16, 3, 24 };
			ofParameter<float> arcHorz{ "Arc Horizontal", 1.0f, 0.0f, 1.0f };
			ofParameter<float> arcVert{ "Arc Vertical", 0.5f, 0.0f, 1.0f };
			ofParameter<glm::vec3> orientation{ "Orientation", glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(glm::two_pi<float>()) };

		protected:
			void rebuildMesh() override;
		};
	}
}