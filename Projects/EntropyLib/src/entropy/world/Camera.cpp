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
			// Build ofEasyCam.
			this->easyCam = std::make_shared<ofEasyCam>();

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
				this->easyCam->setRelativeYAxis(enabled);
			}));
			this->parameterListeners.push_back(fov.newListener([this](float & enabled)
			{
				this->easyCam->setFov(enabled);
			}));
			this->parameterListeners.push_back(nearClip.newListener([this](float & enabled)
			{
				this->easyCam->setNearClip(enabled);
			}));
			this->parameterListeners.push_back(farClip.newListener([this](float & enabled)
			{
				this->easyCam->setFarClip(enabled);
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
			this->easyCam->begin();
			this->easyCam->end();

			// Setup timeline track.
			this->cameraTrack = new ofxTLCameraTrack();
			this->cameraTrack->setCamera(*this->easyCam.get());
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
				delete this->cameraTrack;
				this->cameraTrack = nullptr;
			}
		}

		//--------------------------------------------------------------
		void Camera::reset()
		{
			this->easyCam->setAspectRatio(GetCanvasWidth(this->layout) / GetCanvasHeight(this->layout));
		}

		//--------------------------------------------------------------
		void Camera::update(bool mouseOverGui)
		{
			// TODO: Figure out a better way to do this, with event consumption or something.
			if (mouseOverGui || !this->mouseControl)
			{
				this->easyCam->disableMouseInput();
			}
			else
			{
				this->easyCam->enableMouseInput();
			}
		}

		//--------------------------------------------------------------
		void Camera::resize(ofResizeEventArgs & args)
		{
			this->easyCam->setAspectRatio(args.width / static_cast<float>(args.height));
		}

		//--------------------------------------------------------------
		void Camera::begin()
		{
			this->easyCam->begin(GetCanvasViewport(this->layout));
		}

		//--------------------------------------------------------------
		void Camera::end()
		{
			this->easyCam->end();
		}

		//--------------------------------------------------------------
		std::shared_ptr<ofEasyCam> Camera::getEasyCam()
		{
			return this->easyCam;
		}

		//--------------------------------------------------------------
		void Camera::setControlArea(const ofRectangle & controlArea)
		{
			this->easyCam->setControlArea(controlArea);
		}
		
		//--------------------------------------------------------------
		void Camera::setMouseInputEnabled(bool mouseInputEnabled)
		{
			if (mouseInputEnabled)
			{
				this->easyCam->enableMouseInput();
			}
			else
			{
				this->easyCam->disableMouseInput();
			}
		}

		//--------------------------------------------------------------
		void Camera::setDistanceToTarget(float distanceToTarget)
		{
			this->easyCam->setDistance(distanceToTarget);
		}
		
		//--------------------------------------------------------------
		float Camera::getDistanceToTarget() const
		{
			return this->easyCam->getDistance();
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
		bool Camera::hasParentNode() const
		{
			return (this->parentNode != nullptr);
		}

		//--------------------------------------------------------------
		void Camera::setAttachedToParent(bool attachedToParent)
		{
			if (attachedToParent && this->parentNode)
			{
				this->mouseControl = false;
				this->easyCam->setParent(*this->parentNode.get(), true);
			}
			else
			{
				this->easyCam->clearParent(true);
			}
		}

		//--------------------------------------------------------------
		bool Camera::isAttachedToParent() const
		{
			return (this->easyCam->getParent() != nullptr);
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
		void Camera::copyTransformFromParentNode()
		{
			if (!this->hasParentNode())
			{
				ofLogError(__FUNCTION__) << "No parent node!";
				return;
			}

			const bool wasAttached = this->isAttachedToParent();
			if (wasAttached)
			{
				this->setAttachedToParent(false);
			}
			this->easyCam->setTransformMatrix(this->parentNode->getGlobalTransformMatrix());
			if (wasAttached)
			{
				this->setAttachedToParent(true);
			}
		}

		//--------------------------------------------------------------
		void Camera::addKeyframe()
		{
			this->cameraTrack->addKeyframe();
		}

		//--------------------------------------------------------------
		bool Camera::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ofxPreset::Gui::BeginTree(this->parameters, settings))
			{
				if (this->hasParentNode())
				{
					ofxPreset::Gui::AddParameter(this->attachToParent);
				}
				if (!this->isAttachedToParent())
				{
					ofxPreset::Gui::AddParameter(this->mouseControl);
					ofxPreset::Gui::AddParameter(this->relativeYAxis);
				}
				ofxPreset::Gui::AddParameter(this->fov);
				ofxPreset::Gui::AddRange("Clipping", this->nearClip, this->farClip);

				if (ImGui::Button("Reset"))
				{
					this->reset();
				}
				if (this->hasParentNode())
				{
					ImGui::SameLine();
					if (ImGui::Button("Copy from Parent"))
					{
						this->copyTransformFromParentNode();
					}
				}

				ofxPreset::Gui::EndTree(settings);

				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		void Camera::serialize(nlohmann::json & json)
		{
			auto & jsonGroup = ofxPreset::Serializer::Serialize(json, this->parameters);
			
			ofxPreset::Serializer::Serialize(jsonGroup, *this->easyCam.get(), "ofEasyCam");
		}

		//--------------------------------------------------------------
		void Camera::deserialize(const nlohmann::json & json)
		{
			auto & jsonGroup = ofxPreset::Serializer::Deserialize(json, this->parameters);

			ofxPreset::Serializer::Deserialize(jsonGroup, *this->easyCam.get(), "ofEasyCam");
		}
	}
}