#pragma once

#include "Shape.h"

#include "entropy/render/WireframeFillRenderer.h"

namespace entropy
{
	namespace geom
	{
		class Box
			: public Shape
		{
		public:
			Box();
			~Box();

			void draw(render::WireframeFillRenderer & renderer, ofCamera & camera);
			using Shape::draw;

			ofParameter<float> size{ "Size", 1.0f, 0.0f, 1000.0f };
			ofParameter<float> edgeRatio{ "Edge Ratio", 0.01f, 0.001f, 1.0f };
			ofParameter<int> subdivisions{ "Subdivisions", 1, 1, 10 };

		protected:
			void rebuildMesh() override;
		};
	}
}
