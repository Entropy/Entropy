#include "TravelCamPath.h"

#include "of3dGraphics.h"
#include "ofxSerialize.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace surveys
	{
		//--------------------------------------------------------------
		TravelCamPath::TravelCamPath()
			: travelDistance(0.0f)
			, totalDistance(0.0f)
			, currCloudDistance(0.0f)
			, editPointIdx(-1)
		{}

		//--------------------------------------------------------------
		void TravelCamPath::setup()
		{
			// Initialize parameters.
			ofSetMutuallyExclusive(addPoints, editPoints);
			this->curveResolution.ownListener([this](float &)
			{
				this->buildPath();
			});
			this->editPoints.ownListener([this](bool &)
			{
				this->editPointIdx = -1;
			});

			this->pathOffset.ownListener([this](float &)
			{
				this->currCloudDistance = 0.0f;
				for (int i = 0; i < this->cloudData.size(); ++i)
				{
					this->currCloudDistance += this->pathOffset;
					this->cloudData[i].pathDistance = this->currCloudDistance;
				}
			});

			this->numPlanes.ownListener([this](int &)
			{
				this->generateCloudTextures();
			});
			this->noiseFrequency.ownListener([this](glm::vec4 &)
			{
				this->generateCloudTextures();
			});
			this->colorRampLow.ownListener([this](float &)
			{
				this->generateCloudTextures();
			});
			this->colorRampHigh.ownListener([this](float &)
			{
				this->generateCloudTextures();
			});

			// Populate clouds.
			this->generateCloudTextures();
		}

		//--------------------------------------------------------------
		void TravelCamPath::initGui(ofxPanel & gui)
		{
			gui.add(this->parameters);
			gui.getIntSlider("Num Planes").setUpdateOnReleaseOnly(true);
			gui.getFloatSlider("Noise Frequency").setUpdateOnReleaseOnly(true);
			gui.getFloatSlider("Color Ramp Low").setUpdateOnReleaseOnly(true);
			gui.getFloatSlider("Color Ramp High").setUpdateOnReleaseOnly(true);
		}

		//--------------------------------------------------------------
		void TravelCamPath::addPointToPath(const glm::vec3 & point)
		{
			this->curvePoints.push_back(point);
			this->buildPath();
		}

		//--------------------------------------------------------------
		void TravelCamPath::editNearScreenPoint(const ofCamera & camera, const ofRectangle & viewport, const glm::vec2 & screenPoint)
		{
			auto nearestDist = std::numeric_limits<float>::max();
			auto nearestIdx = -1;
			for (int i = 0; i < this->curvePoints.size(); ++i)
			{
				const auto pt = camera.worldToScreen(this->curvePoints[i], viewport).xy();
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
		void TravelCamPath::generateCloudTextures()
		{
			this->currCloudDistance = 0.0f;
			this->cloudData.resize(this->numPlanes);
			for (size_t i = 0; i < this->numPlanes; ++i) 
			{
				ofFloatPixels pixels;
				auto size = 1024.0f;
				pixels.allocate(size, size, OF_PIXELS_GRAY_ALPHA);
				pixels.set(0.0f);
				for (auto l : pixels.getLines()) 
				{
					auto j = 0;
					for (auto p : l.getPixels()) 
					{
						auto f = ofNoise(j / size * this->noiseFrequency.get().x + i * size, l.getLineNum() / size * this->noiseFrequency.get().x + i * size) / 4.0f +
								 ofNoise(j / size * this->noiseFrequency.get().y + i * size, l.getLineNum() / size * this->noiseFrequency.get().y + i * size) / 4.0f +
								 ofNoise(j / size * this->noiseFrequency.get().z + i * size, l.getLineNum() / size * this->noiseFrequency.get().z + i * size) / 4.0f +
								 ofNoise(j / size * this->noiseFrequency.get().w + i * size, l.getLineNum() / size * this->noiseFrequency.get().w + i * size) / 4.0f;
						f = ofMap(f, colorRampLow, colorRampHigh, 0, 1, true);
						p[0] = f;
						p[1] = f;
						j += 1;
					}
				}
				this->cloudData[i].texture.allocate(pixels);
				this->currCloudDistance += this->pathOffset;
				this->cloudData[i].pathDistance = this->currCloudDistance;
			}
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
				this->currCloudDistance = 0.0f;
				for (int i = 0; i < this->cloudData.size(); ++i)
				{
					this->currCloudDistance += this->pathOffset;
					this->cloudData[i].pathDistance = this->currCloudDistance;
				}
			}
			else if (this->enabled)
			{
				this->travelDistance += this->speed;
			}
			if (this->reset || this->enabled)
			{
				glm::vec3 currPoint;
				glm::vec3 nextPoint;
				//if (this->speed > 0.0f)
				{
					currPoint = this->polyline.getPointAtLength(this->travelDistance);
					const auto nextDistance = this->travelDistance + std::max(0.1f, this->speed.get());
					nextPoint = this->polyline.getPointAtLength(nextDistance);

					this->percent = this->travelDistance / this->totalDistance;
				}
				//else
				//{
				//	currPoint = this->polyline.getPointAtPercent(this->percent);
				//	const auto nextPct = this->percent + 0.01f;
				//	nextPoint = this->polyline.getPointAtPercent(nextPct);
				//}
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

			if (this->renderClouds)
			{
				// Move clouds in front of the camera along the path.
				while (true)
				{
					auto cloud = this->cloudData.front();
					if (cloud.pathDistance == 0.0f || cloud.pathDistance < this->travelDistance)
					{
						this->currCloudDistance += this->pathOffset;
						if (this->currCloudDistance > this->totalDistance)
						{
							// Path is done, nowhere to go.
							break;
						}
						cloud.pathDistance = this->currCloudDistance;
						
						this->cloudData.erase(this->cloudData.begin());
						this->cloudData.push_back(cloud);
						cout << "Pushing cloud plane to distance " << this->currCloudDistance << endl;
					}
					else
					{
						// Planes are in order so we're good to go.
						break;
					}
				}

				const auto camPos = this->camera.getGlobalPosition();
				for (int i = 0; i < this->cloudData.size(); ++i)
				{
					// Update the position.
					this->cloudData[i].position = this->polyline.getPointAtLength(this->cloudData[i].pathDistance);
					
					if (this->cloudData[i].pathDistance > this->travelDistance)
					{
						// Billboard to camera.
						ofNode lookAtNode;
						lookAtNode.setPosition(this->cloudData[i].position);
						lookAtNode.lookAt(camPos);
						this->cloudData[i].transform = lookAtNode.getGlobalTransformMatrix();
					}

					// Set the alpha value.
					if (i == 0)
					{
						float alpha = glm::distance2(camPos, this->cloudData[i].position) / (this->pathOffset * this->pathOffset);
						this->cloudData[i].alpha = ofMap(alpha, 0, 1, 0.1f, 0.8f);
					}
					else
					{
						float dist = glm::distance2(camPos, this->cloudData[i].position);
						this->cloudData[i].alpha = ofMap(dist, 0, pow(this->pathOffset * this->cloudData.size(), 2), 0.8, 0, true);
					}
				}
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
				
				ofSetColor(ofColor::white);
				ofFill();
			}

			if (this->renderClouds)
			{
				for (int i = 0; i < this->cloudData.size(); ++i)
				{
					ofPushMatrix();
					{
						ofMultMatrix(this->cloudData[i].transform);
						
						ofSetColor(ofFloatColor(this->cloudData[i].alpha * this->alphaScalar));
						
						this->cloudData[i].texture.draw(this->planeSize * -0.5f, this->planeSize * -0.5f, this->planeSize, this->planeSize);
					}
					ofPopMatrix();
				}
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
