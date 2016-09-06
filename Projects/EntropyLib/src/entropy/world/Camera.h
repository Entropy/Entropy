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
		static const string kCamerasTimelinePageName = "Cameras";
		
		class Camera
		{
		public:
			Camera();
			~Camera();

			void setup(render::Layout layout, std::shared_ptr<ofxTimeline> timeline);
			void clear();

			void reset();

			void update(bool mouseOverGui);
			void resize(ofResizeEventArgs & args);

			void begin();
			void end();

			std::shared_ptr<ofEasyCam> getEasyCam();
			
			void setControlArea(const ofRectangle & controlArea);
			void setMouseInputEnabled(bool mouseInputEnabled);

			void setDistanceToTarget(float distanceToTarget);
			float getDistanceToTarget() const;

			void setParentNode(std::shared_ptr<ofNode> parentNode);
			void clearParentNode();
			bool hasParentNode() const;

			void setAttachedToParent(bool attachedToParent);
			bool isAttachedToParent() const;

			void addTimelineTrack();
			void removeTimelineTrack();
			bool hasTimelineTrack() const;

			void setLockedToTrack(bool lockedToTrack);
			bool isLockedToTrack() const;

			void copyTransformFromParentNode();

			void addKeyframe();

			bool gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			ofParameter<float> fov{ "FOV", 60, 0, 180 };
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 1000.0f };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.001f, 1000.0f };

			ofParameter<bool> attachToParent{ "Attach to Parent", false };
			ofParameter<bool> mouseControl{ "Mouse Control", true };
			ofParameter<bool> relativeYAxis{ "Relative Y Axis", false };

			ofParameter<bool> useTimelineTrack{ "Use Timeline Track", false };

			ofParameter<float> longitudeSpeed{ "Longitude Speed", 0.0f, -2.0f, 2.0f };
			ofParameter<float> latitudeSpeed{ "Latitude Speed", 0.0f, -2.0f, 2.0f };
			ofParameter<float> radiusSpeed{ "Radius Speed", 0.0f, -2.0f, 2.0f };

			ofParameterGroup parameters{ "Camera",
				fov,
				nearClip, farClip,
				attachToParent,
				mouseControl, relativeYAxis,
				useTimelineTrack,
				longitudeSpeed, latitudeSpeed, radiusSpeed
			};

		protected:
			render::Layout layout;

			std::shared_ptr<ofEasyCam> easyCam;
			std::shared_ptr<ofNode> parentNode;

			std::shared_ptr<ofxTimeline> timeline;
			ofxTLCameraTrack * cameraTrack;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}