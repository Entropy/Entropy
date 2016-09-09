#pragma once

#include "ofParameter.h"
#include "ofVboMesh.h"

namespace entropy
{
	namespace geom
	{
		class Shape
		{
		public:
			enum class CullMode
			{
				Disabled,
				Back,
				Front
			};

			Shape();
			virtual ~Shape();

			virtual void clear();
			virtual void draw();

			const ofVboMesh & getMesh();

			ofParameter<bool> enabled{ "Enabled", true };
			ofParameter<bool> autoDraw{ "Auto Draw", true };
			ofParameter<bool> alphaBlend{ "Alpha Blend", true };
			ofParameter<bool> depthTest{ "Depth Test", true };
			ofParameter<int> cullFace{ "Cull Face", static_cast<int>(CullMode::Back), static_cast<int>(CullMode::Disabled), static_cast<int>(CullMode::Front) };
			ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };
			ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };

			ofParameterGroup parameters{ "Shape",
				enabled,
				autoDraw,
				alphaBlend,
				depthTest,
				cullFace,
				color,
				alpha
			};

		protected:
			virtual void rebuildMesh() = 0;

			ofVboMesh mesh;
			bool meshDirty;
			bool colorDirty;

			std::vector<ofEventListener> paramListeners;
		};
	}
}
