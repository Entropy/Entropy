#pragma once

#include <unordered_set>

#include "ofEasyCam.h"
#include "ofJson.h"
#include "ofParameter.h"
#include "ofPolyline.h"
#include "ofTexture.h"
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

			void setup();

			void addPointToPath(const glm::vec3 & point);
			void editNearScreenPoint(const ofCamera & camera, const ofRectangle & viewport, const glm::vec2 & screenPoint);
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
			ofParameter<bool> reset{ "Reset Position", false };
			ofParameter<float> speed{ "Speed", 0.2f, 0.0f, 0.5f };
			ofParameter<float> percent{ "Percent", 0.0f, 0.0f, 1.0f };
			ofParameter<float> lookAtLerp{ "Look At Lerp", 0.75f, 0.0f, 1.0f };
			ofParameterGroup travel{ "Travel",
				enabled,
				reset,
				speed,
				percent,
				lookAtLerp
			};

			ofParameter<bool> clearPath{ "Clear Path", false };
			ofParameter<float> curveResolution{ "Curve Resolution", 1.0f, 0.1f, 10.0f };
			ofParameter<bool> startPath{ "Start Path", false };
			ofParameter<bool> addPoints{ "Add Points", false };
			ofParameter<bool> editPoints{ "Edit Points", false };
			ofParameter<float> nudgeAmount{ "Nudge Amount", 1.0f, 0.01f, 100.0f };
			ofParameter<bool> debugDraw{ "Debug Draw", false };
			ofParameterGroup edit{ "Edit",
				clearPath,
				curveResolution,
				startPath,
				addPoints,
				editPoints,
				nudgeAmount,
				debugDraw 
			};

			ofParameter<bool> renderClouds{ "Render Clouds", true };
			ofParameter<float> planeSize{ "Plane Size", 512.0f, 128.0f, 8192.0f };
			ofParameter<float> pathOffset{ "Path Offset", 10.0f, 1.0f, 200.0f };
			ofParameter<float> alphaPeak{ "Alpha Peak", 0.5f, 0.0f, 1.0f };
			ofParameter<float> nearDistance{ "Near Distance", 0.0f, 0.0f, 1000.0f };
			ofParameter<float> farDistance{ "Far Distance", 80.0f, 1.0f, 1000.0f };
			ofParameter<float> maxDistance{ "Max Distance", 100.0f, 1.0f, 1000.0f };
			ofParameterGroup clouds{ "Clouds",
				renderClouds,
				planeSize,
				pathOffset,
				alphaPeak,
				nearDistance, farDistance, maxDistance
			};

			ofParameterGroup parameters{ "Travel Cam Path",
				travel,
				edit,
				clouds
			};

		protected:
			void addCurvePointToPolyline(const glm::vec3 & point);

			ofCamera camera;

			float travelDistance;
			float totalDistance;

			vector<ofEventListener> eventListeners;

			glm::vec3 startPosition;
			glm::quat startOrientation;
			std::vector<glm::vec3> curvePoints;
			size_t editPointIdx;

			ofPolyline polyline;

			struct CloudData
			{
				ofTexture texture;
				glm::vec3 position;
				float pathDistance;
				glm::mat4 transform;
			};
			std::vector<CloudData> cloudData;
			float currCloudDistance;
		};
	}
}