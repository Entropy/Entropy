#pragma once

#include "ofMain.h"

namespace entropy
{
	namespace geom
	{
		class Sphere
		{
		public:
			Sphere();
			~Sphere();

			void clear();
			bool update();

			void draw() const;

			const ofVboMesh & getMesh();

			ofParameter<bool> enabled{ "Enabled", true };
			ofParameter<bool> autoDraw{ "Auto Draw", true };
			ofParameter<float> radius{ "Radius", 0.1f, 0.001f, 1.0f };
			ofParameter<int> resolution{ "Resolution", 16, 3, 24 };
			ofParameter<float> arcLength{ "Arc Length", 0.5f, 0.0f, 1.0f };

			ofParameterGroup parameters{ "Sphere",
				enabled,
				autoDraw,
				radius,
				resolution,
				arcLength
			};

		protected:
			void rebuildMesh();

			ofVboMesh mesh;
			bool meshDirty;

			vector<ofEventListener> paramListeners;
		};
	}
}