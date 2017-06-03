#include "TravelCamPath.h"

#include "of3dGraphics.h"
#include "ofxSerialize.h"

namespace entropy
{
	namespace surveys
	{
		//--------------------------------------------------------------
		TravelCamPath::TravelCamPath()
			: travelDistance(0.0f)
			, totalDistance(0.0f)
			, editPointIdx(-1)
		{
			ofSetMutuallyExclusive(addPoints, editPoints);
			this->eventListeners.push_back(this->curveResolution.newListener([this](float &)
			{
				this->buildPath();
			}));
			this->eventListeners.push_back(this->editPoints.newListener([this](bool &)
			{
				this->editPointIdx = -1;
			}));
		}

		//--------------------------------------------------------------
		void TravelCamPath::addPointToPath(const glm::vec3 & point)
		{
			this->curvePoints.push_back(point);
			this->buildPath();
		}

		//--------------------------------------------------------------
		void TravelCamPath::editNearScreenPoint(const ofCamera & camera, const glm::vec2 & screenPoint)
		{
			auto nearestDist = std::numeric_limits<float>::max();
			auto nearestIdx = -1;
			for (int i = 0; i < this->curvePoints.size(); ++i)
			{
				const auto pt = camera.worldToScreen(this->curvePoints[i]).xy();
				auto dist = glm::distance2(pt, screenPoint);
				if (dist < nearestDist) {
					nearestDist = dist;
					nearestIdx = i;
				}
			}
			if (nearestDist < 20.0f)
			{
				this->editPointIdx = nearestIdx;
			}
			else
			{
				this->editPointIdx = -1;
			}
		}

		//--------------------------------------------------------------
		void TravelCamPath::nudgeEditPoint(Nudge nudge)
		{
			if (this->editPointIdx >= this->curvePoints.size()) return;

			auto nudgeVal = glm::vec3(0.0f);
			if (nudge == Nudge::Forward) nudgeVal.z += this->nudgeAmount;
			else if (nudge == Nudge::Back) nudgeVal.z -= this->nudgeAmount;
			else if (nudge == Nudge::Left) nudgeVal.x -= this->nudgeAmount;
			else if (nudge == Nudge::Right) nudgeVal.x += this->nudgeAmount;
			else if (nudge == Nudge::Up) nudgeVal.y += this->nudgeAmount;
			else if (nudge == Nudge::Down) nudgeVal.y -= this->nudgeAmount;

			this->curvePoints[this->editPointIdx] += nudgeVal;
			this->buildPath();
		}

		//--------------------------------------------------------------
		void TravelCamPath::buildPath()
		{
			this->polyline.clear();

			// Start at the camera position.
			const auto startPoint = this->startPosition;
			// End at the origin.
			const auto endPoint = glm::vec3(0.0f);
			
			this->polyline.addVertex(startPoint);

			if (!this->curvePoints.empty())
			{
				this->polyline.curveTo(startPoint);
				this->polyline.curveTo(startPoint);

				// Add all the galaxy points.
				for (int i = 0; i < this->curvePoints.size(); ++i)
				{
					const auto & currPoint = this->curvePoints[i];
					this->addCurvePointToPolyline(currPoint);
				}

				// Add a couple of points in back to get a smooth finish.
				this->addCurvePointToPolyline(endPoint);
				this->addCurvePointToPolyline(endPoint);
			}

			this->polyline.lineTo(endPoint);

			this->totalDistance = this->polyline.getPerimeter();

			this->reset = true;
		}

		//--------------------------------------------------------------
		void TravelCamPath::addCurvePointToPolyline(const glm::vec3 & point)
		{
			auto tempPolyline = this->polyline;
			tempPolyline.curveTo(point);
			float segmentLength = tempPolyline.getPerimeter() - this->polyline.getPerimeter();
			int resolution = segmentLength * this->curveResolution;
			this->polyline.curveTo(point, resolution);
		}

		//--------------------------------------------------------------
		void TravelCamPath::update(const ofCamera & camera)
		{	
			if (this->startPath)
			{
				this->copyCamera(camera, true);
				this->buildPath();
				this->startPath = false;
			}
			
			if (this->clearPath)
			{
				this->curvePoints.clear();
				this->buildPath();
				this->clearPath = false;
			}

			if (this->reset)
			{
				this->travelDistance = 0.0f;
			}
			else if (this->enabled)
			{
				this->travelDistance += this->speed;
			}
			if (this->reset || this->enabled)
			{
				const auto currPoint = polyline.getPointAtLength(this->travelDistance);
				const auto nextDistance = this->travelDistance + this->speed;
				const auto nextPoint = polyline.getPointAtLength(nextDistance);
				this->camera.setPosition(currPoint);

				const auto xAxis = glm::normalize(this->camera.getXAxis());
				const auto upDir = glm::normalize(glm::cross(xAxis, glm::normalize(nextPoint - currPoint)));

				if (this->lookAtLerp == 1.0f)
				{
					this->camera.lookAt(nextPoint, upDir);
				}
				else
				{
					const auto zAxis = glm::normalize(this->camera.getGlobalPosition() - nextPoint);
					if (glm::length(zAxis) > 0.0f) 
					{
						const auto xAxis = glm::normalize(glm::cross(upDir, zAxis));
						const auto yAxis = glm::cross(zAxis, xAxis);
						glm::mat4 m;
						m[0] = glm::vec4(xAxis, 0.f);
						m[1] = glm::vec4(yAxis, 0.f);
						m[2] = glm::vec4(zAxis, 0.f);
						const auto targetOrientation = glm::mix(this->camera.getGlobalOrientation(), glm::toQuat(m), this->lookAtLerp.get());
						this->camera.setGlobalOrientation(targetOrientation);
					}
				}

				this->reset = false;
			}
		}

		//--------------------------------------------------------------
		void TravelCamPath::draw() const
		{
			if (this->debugDraw)
			{
				ofNoFill();
				ofSetColor(ofColor::crimson);
				this->polyline.draw();

				for (int i = 0; i < this->curvePoints.size(); ++i)
				{
					if (i == this->editPointIdx)
					{
						ofSetColor(ofColor::green.getLerped(ofColor::red, sin(ofGetFrameNum() * 0.1f) * 0.5f + 0.5f));
					}
					else
					{
						ofSetColor(ofColor::green);
					}
					ofDrawBox(this->curvePoints[i], 3.0f);
				}

				//ofSetColor(ofColor::purple);
				//for (auto & v : this->polyline.getVertices())
				//{
				//	ofDrawBox(v, 1.0f / this->curveResolution);
				//}

				ofSetColor(ofColor::blue);
				this->camera.draw();
				ofFill();
			}
		}

		//--------------------------------------------------------------
		void TravelCamPath::copyCamera(const ofCamera & camera, bool copyTransform)
		{
			const auto savedPosition = this->camera.getGlobalPosition();
			const auto savedOrientation = this->camera.getGlobalOrientation();
			this->camera = camera;
			if (!copyTransform)
			{
				this->camera.setPosition(savedPosition);
				this->camera.setOrientation(savedOrientation);
			}
			this->startPosition = this->camera.getGlobalPosition();
			this->startOrientation = this->camera.getGlobalOrientation();
		}

		//--------------------------------------------------------------
		void TravelCamPath::resetCamera()
		{
			this->camera.setPosition(this->startPosition);
			this->camera.setOrientation(this->startOrientation);
		}

		//--------------------------------------------------------------
		ofCamera & TravelCamPath::getCamera()
		{
			return this->camera;
		}

		//--------------------------------------------------------------
		void TravelCamPath::serialize(nlohmann::json & json)
		{
			// Reset the camera first.
			cout << "pree cam pos " << this->camera.getGlobalPosition() << endl;
			this->resetCamera();
			cout << "post cam pos " << this->camera.getGlobalPosition() << endl;

			auto & jsonGroup = json["travelCamPath"];
			ofSerialize(jsonGroup, this->camera, "camera");
			ofSerialize(jsonGroup, this->curvePoints, "curvePoints");
		}
		
		//--------------------------------------------------------------
		void TravelCamPath::deserialize(const nlohmann::json & json)
		{
			if (json.count("travelCamPath"))
			{
				this->curvePoints.clear();

				const auto & jsonGroup = json["travelCamPath"];
				ofDeserialize(jsonGroup, this->camera, "camera");
				ofDeserialize(jsonGroup, this->curvePoints, "curvePoints");

				this->startPosition = this->camera.getGlobalPosition();
				this->startOrientation = this->camera.getGlobalOrientation();

				this->buildPath();
			}
		}
	}
}
