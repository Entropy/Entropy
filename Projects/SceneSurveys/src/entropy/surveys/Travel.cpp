#include "Travel.h"

#include "of3dGraphics.h"

namespace entropy
{
	namespace surveys
	{
		//--------------------------------------------------------------
		Travel::Travel()
		{
			this->eventListeners.push_back(this->enabled.newListener([this](bool &)
			{
				this->prevTargetIndex = -1;
				this->travelLog.clear();
			}));
		}

		//--------------------------------------------------------------
		void Travel::update(DataSet & dataSet, ofEasyCam & camera, const glm::mat4 & worldTransform)
		{
			if (this->enabled)
			{
				// Get the data target.
				int currTargetIndex = dataSet.getTargetIndex();
				this->currCamPos = camera.getPosition();
				if (glm::length(this->currCamPos) > this->camCutoff &&
					(this->prevTargetIndex == currTargetIndex ||
						this->travelLog.find(currTargetIndex) == this->travelLog.end()))
				{
					// Camera is far enough from the origin.
					// Target is either the same as previous frame, or it's never been visited on this travel.
					this->currTargetPos = (worldTransform * glm::vec4(dataSet.getTargetPosition(), 1.0f)).xyz();
					this->prevTargetIndex = currTargetIndex;
					this->travelLog.insert(currTargetIndex);
					//cout << "[ofApp] Travel target index = " << this->prevTargetIndex << endl;
				}
				else
				{
					// Using default target at origin.
					this->prevTargetIndex = -1;
					//cout << "[ofApp] Default target index = " << this->prevTargetIndex << endl;
				}

				const auto toTarget = this->currTargetPos - this->currCamPos;
				const auto lerpedDir = glm::normalize(glm::mix(camera.getLookAtDir(), glm::normalize(toTarget), this->lookAtLerp.get()));
				const auto targetDist = glm::length(toTarget);

				//camera.lookAt(camPos + lerpedDir * targetDist);
				if (this->lookAtTarget)
				{
					this->testNode.lookAt(this->currTargetPos);
				//	camera.lookAt(this->currTargetPos);
				}
				auto lerpPos = glm::mix(this->currCamPos, this->currTargetPos, this->moveLerp.get());
				if (this->maxSpeed > 0.0f && this->maxSpeed < glm::distance(this->currCamPos, lerpPos))
				{
					// Clamp the new position if it's too big of a jump.
					lerpPos = this->currCamPos + glm::normalize(toTarget) * this->maxSpeed.get();
				}
				if (this->moveToTarget)
				{
					camera.setPosition(lerpPos);
				}
			}

			if (this->placeNode)
			{
				auto distance = glm::length(camera.getPosition()) - 10.0f;
				auto direction = glm::normalize(camera.getPosition());
				this->testNode.resetTransform();
				this->testNode.setPosition(direction * distance);

				this->placeNode = false;
			}

			if (this->lookAtOrigin)
			{
				this->testNode.lookAt(glm::vec3(0), glm::vec3(0, 1, 0));

				this->lookAtOrigin = false;
			}
		}

		//--------------------------------------------------------------
		void Travel::draw(DataSet::SharedParams & sharedParams) const
		{
			if (this->debugDraw)
			{
				if (this->prevTargetIndex != -1)
				{
					ofNoFill();
					
					ofSetColor(ofColor::blue);
					ofPushMatrix();
					ofMultMatrix(this->testNode.getGlobalTransformMatrix());
					{
						ofDrawBox(glm::vec3(0), sharedParams.target.lockDistance * 2.0f);
					}
					ofPopMatrix();

					ofSetColor(ofColor::green);
					ofDrawLine(this->testNode.getPosition(), this->currTargetPos);
					ofSetColor(ofColor::red);
					ofDrawLine(glm::vec3(0), this->currTargetPos);
					ofDrawBox(this->currTargetPos, sharedParams.target.lockDistance * 2.0f);
					ofFill();
				}
			}
		}
	}
}
