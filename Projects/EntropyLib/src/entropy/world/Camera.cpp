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
			
			this->parameterListeners.push_back(attachToParent.newListener([this](bool & enabled)
			{
				this->setAttachedToParent(enabled);
			}));
			this->parameterListeners.push_back(mouseControl.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->attachToParent = false;
				}
				this->setMouseInputEnabled(enabled);
			}));
			this->parameterListeners.push_back(relativeYAxis.newListener([this](bool & enabled)
			{
				this->easyCam->setRelativeYAxis(enabled);
			}));

			this->parameterListeners.push_back(useTimelineTrack.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->addTimelineTrack();
				}
				else
				{
					this->removeTimelineTrack();
				}
			}));
		}
		
		//--------------------------------------------------------------
		Camera::~Camera()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Camera::setup(render::Layout layout, std::shared_ptr<ofxTimeline> timeline)
		{
			this->clear();

			this->timeline = timeline;

			this->layout = layout;
			const auto name = (this->layout == render::Layout::Back ? "Camera Back" : "Camera Front");
			this->parameters.setName(name);

			// Make sure the ofEasyCam is up to date.
			this->easyCam->begin();
			this->easyCam->end();
		}
		
		//--------------------------------------------------------------
		void Camera::clear()
		{
			this->parentNode.reset();

			this->removeTimelineTrack();
			this->timeline.reset();
		}

		//--------------------------------------------------------------
		void Camera::reset()
		{
			this->easyCam->setAspectRatio(GetCanvasWidth(this->layout) / GetCanvasHeight(this->layout));
			this->easyCam->reset();
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

			if (!this->attachToParent && !this->useTimelineTrack && !this->easyCam->isInMotion())
			{
				// Get current values.
				const auto position = this->easyCam->getPosition();
				glm::vec3 polarCoordinates;
				polarCoordinates.z = glm::length(position);
				polarCoordinates.y = acosf(position.y / polarCoordinates.z) - glm::half_pi<float>();
				polarCoordinates.x = atan2f(position.x, position.z);

				// Calculate offsets.
				polarCoordinates.x += this->longitudeSpeed;
				polarCoordinates.y += this->latitudeSpeed;
				polarCoordinates.z += this->radiusSpeed;

				// Apply new orientation.
				this->easyCam->orbitRad(polarCoordinates.x, polarCoordinates.y, polarCoordinates.z);
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
				this->useTimelineTrack = false;
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
		void Camera::addTimelineTrack()
		{
			if (!this->timeline)
			{
				ofLogError(__FUNCTION__) << "No timeline set, call setup() first!";
				return;
			}

			if (this->cameraTrack)
			{
				ofLogWarning(__FUNCTION__) << "Camera track already exists.";
				return;
			}

			// Add Page if it doesn't already exist.
			if (!this->timeline->hasPage(kCamerasTimelinePageName))
			{
				this->timeline->addPage(kCamerasTimelinePageName);
			}
			this->timeline->setCurrentPage(kCamerasTimelinePageName);

			const auto trackName = this->parameters.getName();

			this->cameraTrack = new ofxTLCameraTrack();
			this->cameraTrack->setCamera(*this->easyCam.get());
			this->cameraTrack->setXMLFileName(this->timeline->nameToXMLName(trackName));
			this->timeline->addTrack(trackName, this->cameraTrack);
			this->cameraTrack->setDisplayName(trackName);
			this->cameraTrack->lockCameraToTrack = false;
		}

		//--------------------------------------------------------------
		void Camera::removeTimelineTrack()
		{
			if (!this->timeline)
			{
				//ofLogWarning(__FUNCTION__) << "No timeline set.";
				return;
			}

			if (!this->cameraTrack)
			{
				//ofLogWarning(__FUNCTION__) << "Camera track does not exist.";
				return;
			}

			this->timeline->removeTrack(this->cameraTrack);
			delete this->cameraTrack;
			this->cameraTrack = nullptr;
		}

		//--------------------------------------------------------------
		bool Camera::hasTimelineTrack() const
		{
			return (this->cameraTrack != nullptr);
		}

		//--------------------------------------------------------------
		void Camera::setLockedToTrack(bool lockedToTrack)
		{
			if (!this->hasTimelineTrack()) return;

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
			if (!this->hasTimelineTrack()) return false;
			
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
				ofxPreset::Gui::AddParameter(this->fov);
				ofxPreset::Gui::AddRange("Clipping", this->nearClip, this->farClip);
				
				if (this->hasParentNode())
				{
					ofxPreset::Gui::AddParameter(this->attachToParent);
				}
				if (!this->isAttachedToParent())
				{
					ofxPreset::Gui::AddParameter(this->mouseControl);
					ofxPreset::Gui::AddParameter(this->relativeYAxis);
				}
				
				if (ImGui::Button("Reset"))
				{
					this->reset();
				}
				ImGui::SameLine();
				if (ImGui::Button("Set to Origin"))
				{
					this->easyCam->setPosition(glm::vec3(0.0f));
				}
				if (this->hasParentNode())
				{
					ImGui::SameLine();
					if (ImGui::Button("Copy from Parent"))
					{
						this->copyTransformFromParentNode();
					}
				}

				if (!this->isAttachedToParent())
				{
					ofxPreset::Gui::AddParameter(this->useTimelineTrack);

					if (!this->hasTimelineTrack())
					{
						ImGui::Text("Orbit");
						ofxPreset::Gui::AddParameter(this->longitudeSpeed);
						ofxPreset::Gui::AddParameter(this->latitudeSpeed);
						ofxPreset::Gui::AddParameter(this->radiusSpeed);
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