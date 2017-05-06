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

			virtual void begin();
			virtual void end();

			virtual void draw();

			const ofVboMesh & getMesh();

			ofParameter<bool> enabled{ "Enabled", true };
			ofParameter<bool> autoDraw{ "Auto Draw", true };
			ofParameter<int> blendMode{ "Blend Mode", static_cast<int>(OF_BLENDMODE_ALPHA), static_cast<int>(OF_BLENDMODE_DISABLED), static_cast<int>(OF_BLENDMODE_SCREEN) };
			ofParameter<bool> depthTest{ "Depth Test", false };
			ofParameter<int> cullFace{ "Cull Face", static_cast<int>(CullMode::Back), static_cast<int>(CullMode::Disabled), static_cast<int>(CullMode::Front) };
			ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };
			ofParameter<float> alpha{ "Alpha", 1.0f, 0.0f, 1.0f };

			ofParameterGroup parameters{ "Shape",
				enabled,
				autoDraw,
				blendMode,
				depthTest,
				cullFace,
				color,
				alpha
			};

		protected:
			typedef enum
			{
				Front = 0x000001,
				Back = 0x000010,
				Right = 0x000100,
				Left = 0x001000,
				Top = 0x010000,
				Bottom = 0x100000,

				All = 0x111111
			} Face;

			void addEdge(const glm::vec3 & center, const glm::vec3 & dimensions, int faces);
			
			virtual void rebuildMesh() = 0;

			ofVboMesh mesh;
			bool meshDirty;
			bool colorDirty;

			std::vector<ofEventListener> paramListeners;
		};
	}
}
