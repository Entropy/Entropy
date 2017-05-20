#pragma once

#include <unordered_set>

#include "ofEasyCam.h"
#include "ofParameter.h"
#include "ofVectorMath.h"

#include "entropy/surveys/DataSet.h"

namespace entropy
{
	namespace surveys
	{
		class Travel
		{
		public:
			Travel();

			void update(DataSet & dataSet, ofEasyCam & camera, const glm::mat4 & worldTransform);
			void draw(DataSet::SharedParams & sharedParams) const;

			ofParameter<bool> enabled{ "Enable Travel", false };
			ofParameter<bool> debugDraw{ "Debug Draw", false };
			ofParameter<float> camCutoff{ "Cam Cutoff", 1.0f, 0.0f, 1000.0f };
			ofParameter<float> lookAtLerp{ "Look At Lerp", 0.2f, 0.0f, 1.0f };
			ofParameter<float> moveLerp{ "Move Lerp", 0.1f, 0.0f, 1.0f };
			ofParameter<float> maxSpeed{ "Max Speed", 10.0f, 0.0f, 100.0f };

			ofParameter<bool> placeNode{ "Place Node", false };
			ofParameter<bool> lookAtTarget{ "Look at Target", false };
			ofParameter<bool> lookAtOrigin{ "Look at Origin", false }; 
			ofParameter<bool> moveToTarget{ "Move to Target", false };

			ofParameterGroup parameters{ "Travel",
				enabled,
				debugDraw,
				camCutoff,
				lookAtLerp,
				moveLerp,
				maxSpeed,
				placeNode,
				lookAtTarget,
				lookAtOrigin,
				moveToTarget
			};

		protected:
			glm::vec3 currCamPos;
			glm::vec3 currTargetPos;
			int prevTargetIndex;
			std::unordered_set<int> travelLog;

			ofNode testNode;

			vector<ofEventListener> eventListeners;
		};
	}
}