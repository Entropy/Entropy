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
		class Camera
		{
		public:
			Camera();
			~Camera();

			void setup(render::Layout layout, ofxTimeline & timeline);
			void clear(ofxTimeline & timeline);

			void reset();

			void update(bool mouseOverGui);
			void resize(ofResizeEventArgs & args);

			void begin();
			void end();

			std::shared_ptr<ofEasyCam> getEasyCam();
			
			void setControlArea(const ofRectangle & controlArea);
			void setMouseInputEnabled(bool mouseInputEnabled);

			void setParentNode(std::shared_ptr<ofNode> parentNode);
			void clearParentNode();
			bool hasParentNode() const;

			void setAttachedToParent(bool attachedToParent);
			bool isAttachedToParent() const;

			void setLockedToTrack(bool lockedToTrack);
			bool isLockedToTrack() const;

			void copyTransformFromParentNode();

			void addKeyframe();

			bool gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			ofParameter<bool> attachToParent{ "Attach to Parent", false };
			ofParameter<bool> mouseControl{ "Mouse Control", true };
			ofParameter<bool> relativeYAxis{ "Relative Y Axis", false };
			ofParameter<float> fov{ "FOV", 60, 0, 180 };
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 1000.0f };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 0.001f, 1000.0f };

			ofParameterGroup parameters{ "Camera",
				attachToParent,
				mouseControl,
				relativeYAxis,
				fov,
				nearClip,
				farClip
			};

		protected:
			render::Layout layout;

			std::shared_ptr<ofEasyCam> easyCam;
			std::shared_ptr<ofNode> parentNode;
			
			ofxTLCameraTrack * cameraTrack;

			std::vector<ofEventListener> parameterListeners;
		};
	}
}