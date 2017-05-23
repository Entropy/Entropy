#pragma once

#include <unordered_set>

#include "ofEasyCam.h"
#include "ofJson.h"
#include "ofParameter.h"
#include "ofPolyline.h"
#include "ofVectorMath.h"

namespace entropy
{
	namespace surveys
	{
		enum class Nudge
		{
			Forward,
			Back,
			Left,
			Right,
			Up,
			Down
		};
		
		class TravelCamPath
		{
		public:
			TravelCamPath();

			void addPointToPath(const glm::vec3 & point);
			void editNearScreenPoint(const ofCamera & camera, const glm::vec2 & screenPoint);
			void nudgeEditPoint(Nudge nudge);
			void buildPath();

			void update(const ofCamera & camera);
			void draw() const;

			void copyCamera(const ofCamera & camera, bool copyTransform);
			void resetCamera();
			ofCamera & getCamera();

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			ofParameter<bool> enabled{ "Enable Travel", false };
			ofParameter<bool> lookThrough{ "Look Through", true };
			ofParameter<bool> reset{ "Reset Position", false };
			ofParameter<float> speed{ "Speed", 1.0f, 0.0f, 100.0f };
			ofParameterGroup travel{ "Travel",
				enabled,
				lookThrough,
				reset,
				speed 
			};

			ofParameter<bool> clearPath{ "Clear Path", false };
			ofParameter<bool> startPath{ "Start Path", false };
			ofParameter<bool> addPoints{ "Add Points", false };
			ofParameter<bool> editPoints{ "Edit Points", false };
			ofParameter<float> nudgeAmount{ "Nudge Amount", 1.0f, 0.01f, 100.0f };
			ofParameter<bool> debugDraw{ "Debug Draw", false };
			ofParameterGroup edit{ "Edit",
				clearPath,
				startPath,
				addPoints,
				editPoints,
				nudgeAmount,
				debugDraw 
			};

			ofParameterGroup parameters{ "Travel Cam Path",
				travel,
				edit
			};

		protected:
			ofCamera camera;

			float travelDistance;
			float totalDistance;

			vector<ofEventListener> eventListeners;

			glm::vec3 startPosition;
			glm::quat startOrientation;
			std::vector<glm::vec3> curvePoints;
			size_t editPointIdx;

			ofPolyline polyline;
		};
	}
}