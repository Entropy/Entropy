#pragma once

#include "ofMain.h"

namespace entropy
{
	namespace geom
	{
		class Box
		{
		public:
			Box();
			~Box();

			void clear();
			bool update();

			const ofVboMesh & getMesh();

			ofParameter<float> size{ "Size", 20.0f, 0.0f, 200.0f };
			ofParameter<float> edgeWidth{ "Edge Width", 1.0f, 0.01f, 10.0f };
			ofParameter<int> subdivisions{ "Subdivisions", 1, 1, 10 };

		protected:
			typedef enum
			{
				Front  = 0x000001,
				Back   = 0x000010, 
				Right  = 0x000100,
				Left   = 0x001000,
				Top    = 0x010000,
				Bottom = 0x100000,

				All    = 0x111111
			} Face;

			void addEdge(const glm::vec3 & center, const glm::vec3 & dimensions, int faces);

			ofVboMesh mesh;
			bool meshDirty;

			vector<ofEventListener> paramListeners;
		};
	}
}