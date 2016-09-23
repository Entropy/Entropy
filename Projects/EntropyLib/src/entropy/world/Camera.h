#pragma once

#include "ofEasyCam.h"
#include "ofParameter.h"

#include "ofxPreset.h"
#include "ofxTimeline.h"
#include "ofxTLCameraTrack.h"

#include "entropy/render/Layout.h"

namespace entropy
{
	namespace world
	{
		static const string CameraTimelinePageName = "Cameras";
		
		class Camera
		{
		public:
			struct Settings
			{				
				Settings();
				
				float fov;
				float nearClip;
				float farClip;
				glm::vec3 position;
				glm::quat orientation;
			};

			Camera();
			~Camera();

			void setup(render::Layout layout, std::shared_ptr<ofxTimeline> timeline);
			void clear();

			void reset(bool transform);

			void update(bool mouseOverGui);
			void resize(ofResizeEventArgs & args);

			void begin();
			void end();

			void applySettings(const Settings & settings);
			Settings fetchSettings();

			ofEasyCam & getEasyCam();

			glm::mat4 getTransform() const;
			
			void setControlArea(const ofRectangle & controlArea);
			void setMouseInputEnabled(bool mouseInputEnabled);

			void setDistanceToTarget(float distanceToTarget);
			float getDistanceToTarget() const;

			void setParent(std::shared_ptr<Camera> parent);
			void clearParent();
			bool hasParent() const;

			void setAttachedToParent(bool attachedToParent);
			bool isAttachedToParent() const;

			void copyTransformFromParent();

			void addTimelineTrack();
			void removeTimelineTrack();
			bool hasTimelineTrack() const;

			void setLockedToTrack(bool lockedToTrack);
			bool isLockedToTrack() const;

			void addKeyframe();

			bool gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			ofParameter<float> fov{ "FOV", 60, 0, 180 };
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 1000.0f };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.001f, 1000.0f };

			ofParameter<bool> attachToParent{ "Attach to Parent", false };
			ofParameter<bool> inheritsSettings{ "Inherits Settings", false };
			ofParameter<bool> mouseControl{ "Mouse Control", true };
			ofParameter<bool> relativeYAxis{ "Relative Y Axis", false };

			ofParameter<float> tiltSpeed{ "Tilt Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> panSpeed{ "Pan Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> rollSpeed{ "Roll Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> dollySpeed{ "Dolly Speed", 0.0f, -10.0f, 10.0f };

			ofParameter<bool> tumbleOverride{ "Tumble Override", false };

			ofParameterGroup parameters{ "Camera",
				fov,
				nearClip, farClip,
				attachToParent,
				inheritsSettings,
				mouseControl, relativeYAxis,
				tiltSpeed, panSpeed, rollSpeed, dollySpeed,
				tumbleOverride
			};

		protected:
			render::Layout layout;

			ofEasyCam easyCam;

			glm::vec3 tumbleOffset;
			float dollyOffset;

			void beginTumbleOverride();
			
			struct TweenParams
			{
				glm::vec3 origin;
				glm::vec3 target;
				glm::vec3 tumble;
				float startTime;
				float currPct;
				float totalDuration;
			};
			TweenParams overrideParams;

			std::shared_ptr<Camera> parent;

			std::shared_ptr<ofxTimeline> timeline;
			ofxTLCameraTrack * cameraTrack;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}