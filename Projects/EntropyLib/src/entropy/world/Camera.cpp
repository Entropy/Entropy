#include "Camera.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace world
	{
		//--------------------------------------------------------------
		Camera::Camera()
			: cameraTrack(nullptr)
		{
			// Set parameter listeners.
			this->parameterListeners.push_back(mouseControl.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->attachToParent = false;
				}
				this->setMouseInputEnabled(enabled);
			}));
			this->parameterListeners.push_back(attachToParent.newListener([this](bool & enabled)
			{
				this->setAttachedToParent(enabled);
			}));
			this->parameterListeners.push_back(relativeYAxis.newListener([this](bool & enabled)
			{
				this->easyCam.setRelativeYAxis(enabled);
			}));
			this->parameterListeners.push_back(fov.newListener([this](float & enabled)
			{
				this->easyCam.setFov(enabled);
			}));
			this->parameterListeners.push_back(nearClip.newListener([this](float & enabled)
			{
				this->easyCam.setNearClip(enabled);
			}));
			this->parameterListeners.push_back(farClip.newListener([this](float & enabled)
			{
				this->easyCam.setFarClip(enabled);
			}));
		}
		
		//--------------------------------------------------------------
		Camera::~Camera()
		{

		}

		//--------------------------------------------------------------
		void Camera::setup(render::Layout layout, ofxTimeline & timeline)
		{
			this->clear(timeline);

			this->layout = layout;
			const auto name = (this->layout == render::Layout::Back ? "Camera Back" : "Camera Front");
			this->parameters.setName(name);

			// Make sure the ofEasyCam is up to date.
			this->easyCam.begin();
			this->easyCam.end();

			// Setup timeline track.
			this->cameraTrack = new ofxTLCameraTrack();
			this->cameraTrack->setCamera(this->easyCam);
			this->cameraTrack->setXMLFileName(timeline.nameToXMLName(name));
			timeline.addTrack(name, this->cameraTrack);
			this->cameraTrack->setDisplayName(name);
			this->cameraTrack->lockCameraToTrack = false;
		}
		
		//--------------------------------------------------------------
		void Camera::clear(ofxTimeline & timeline)
		{
			this->parentNode.reset();

			if (this->cameraTrack)
			{
				timeline.removeTrack(this->cameraTrack);
				this->cameraTrack = nullptr;
			}
		}

		//--------------------------------------------------------------
		void Camera::reset()
		{
			this->easyCam.setAspectRatio(GetCanvasWidth(this->layout) / GetCanvasHeight(this->layout));
		}

		//--------------------------------------------------------------
		void Camera::update(bool mouseOverGui)
		{
			if (mouseOverGui || !this->mouseControl)
			{
				this->easyCam.disableMouseInput();
			}
			else
			{
				this->easyCam.enableMouseInput();
			}
		}

		//--------------------------------------------------------------
		void Camera::resize(ofResizeEventArgs & args)
		{
			this->easyCam.setAspectRatio(args.width / static_cast<float>(args.height));
		}

		//--------------------------------------------------------------
		void Camera::setControlArea(const ofRectangle & controlArea)
		{
			this->easyCam.setControlArea(controlArea);
		}
		
		//--------------------------------------------------------------
		void Camera::setMouseInputEnabled(bool mouseInputEnabled)
		{
			if (mouseInputEnabled)
			{
				this->easyCam.enableMouseInput();
			}
			else
			{
				this->easyCam.disableMouseInput();
			}
		}

		//--------------------------------------------------------------
		void Camera::setParentNode(std::shared_ptr<ofNode> parentNode)
		{
			this->parentNode = parentNode;
		}

		//--------------------------------------------------------------
		void Camera::clearParentNode()
		{
			this->parentNode.reset();
		}

		//--------------------------------------------------------------
		void Camera::setAttachedToParent(bool attachedToParent)
		{
			if (attachedToParent && this->parentNode)
			{
				this->mouseControl = false;
				this->easyCam.setParent(*this->parentNode.get(), true);
			}
			else
			{
				this->easyCam.clearParent(true);
			}
		}

		//--------------------------------------------------------------
		void Camera::setLockedToTrack(bool lockedToTrack)
		{
			if (this->attachToParent)
			{
				// Don't lock when attached.
				this->cameraTrack->lockCameraToTrack = false;
			}
			else
			{
				this->cameraTrack->lockCameraToTrack = lockedToTrack;
			}
		}
		
		//--------------------------------------------------------------
		bool Camera::isLockedToTrack() const
		{
			return this->cameraTrack->lockCameraToTrack;
		}

		//--------------------------------------------------------------
		void Camera::addKeyframe()
		{
			this->cameraTrack->addKeyframe();
		}

		//--------------------------------------------------------------
		void Camera::serialize(nlohmann::json & json)
		{

		}

		//--------------------------------------------------------------
		void Camera::deserialize(const nlohmann::json & json)
		{

		}
	}
}