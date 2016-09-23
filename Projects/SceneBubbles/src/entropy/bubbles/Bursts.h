#pragma once

#include "ofMesh.h"
#include "ofParameter.h"
#include "ofShader.h"
#include "ofVboMesh.h"
#include "ofVectorMath.h"

namespace entropy
{
	namespace bubbles
	{
		enum ExtraAttribute
		{
			Age = 5
		};

		class Bursts
		{
		public:
			Bursts();
			~Bursts();

			void init();
			void exit();

			void reset();

			void addDrop(const glm::vec3 & center, float radius);

			void update(double dt);
			void draw(float alpha = 1.0f);

			ofParameter<bool> enabled{ "Enabled", true };
			ofParameter<int> resolution{ "Resolution", 8, 4, 64 };
			ofParameter<ofFloatColor> color{ "Color", ofFloatColor::white };
			ofParameter<float> maxAge{ "Max Age", 3.0f, 1.0f, 10.0f };
			ofParameter<float> forceMultiplier{ "Force Multiplier", 0.5f, 0.1f, 1.0f };
			ofParameter<float> forceRandom{ "Force Random", 0.2f, 0.0f, 1.0f };
			ofParameter<float> worldBounds{ "World Bounds", 800.0f, 512.0f, 1920.0f };
			ofParameter<float> minDistance{ "Min Distance", 0.08f, 0.01f, 1.0f };
			ofParameter<float> maxDistance{ "Max Distance", 0.25f, 0.01f, 1.0f };
			ofParameter<int> maxLinks{ "Max Links", 8, 2, 64 };

			ofParameterGroup parameters{ "Bursts",
				enabled,
				resolution,
				color,
				maxAge,
				forceMultiplier, forceRandom,
				worldBounds,
				minDistance, maxDistance,
				maxLinks
			};

		protected:
			ofVbo vbo;
			ofShader shader;

			ofMesh unitSphere;

			std::vector<glm::vec3> pos;
			std::vector<glm::vec3> vel;
			std::vector<glm::vec3> acc;

			std::vector<float> age;
			std::vector<int> links;
			std::vector<size_t> zombies;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}
