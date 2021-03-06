#include "DataSet.h"

#include "ofCamera.h"
#include "ofxHDF5.h"

namespace entropy
{
	namespace surveys
	{
		//--------------------------------------------------------------
		DataSet::DataSet()
		{
			this->clear();
		}
		
		//--------------------------------------------------------------
		DataSet::~DataSet()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void DataSet::setup(const std::string & name, const std::string & format, size_t startIdx, size_t count, const std::string & particleType)
		{
			this->parameters.setName(name);

			this->clear();

			auto radiusChanged = [this](const float &) 
			{
				this->mappedRadiusRange = glm::vec3(ofMap(parameters.cutRadius, 0.0f, 1.0f, this->minRadius, this->maxRadius),
													ofMap(parameters.minRadius, 0.0f, 1.0f, this->minRadius, this->maxRadius),
													ofMap(parameters.maxRadius, 0.0f, 1.0f, this->minRadius, this->maxRadius));
			};
			this->paramListeners.push_back(this->parameters.cutRadius.newListener(radiusChanged));
			this->paramListeners.push_back(this->parameters.minRadius.newListener(radiusChanged));
			this->paramListeners.push_back(this->parameters.maxRadius.newListener(radiusChanged));

			auto latitudeChanged = [this](const float &)
			{
				static const auto kLatitudeMin = -HALF_PI;
				static const auto kLatitudeMax = HALF_PI;

				this->mappedLatitudeRange = glm::vec2(ofMap(parameters.minLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax),
													  ofMap(parameters.maxLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
			};
			this->paramListeners.push_back(this->parameters.minLatitude.newListener(latitudeChanged));
			this->paramListeners.push_back(this->parameters.maxLatitude.newListener(latitudeChanged));

			auto longitudeChanged = [this](const float &)
			{
				static const auto kLongitudeMin = 0;
				static const auto kLongitudeMax = TWO_PI;

				this->mappedLongitudeRange = glm::vec2(ofMap(parameters.minLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax),
													   ofMap(parameters.maxLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
			};
			this->paramListeners.push_back(this->parameters.minLongitude.newListener(longitudeChanged));
			this->paramListeners.push_back(this->parameters.maxLongitude.newListener(longitudeChanged));

			// Load the data one fragment at a time.
			for (int i = 0; i < count; ++i)
			{
				char filePath[512];
				sprintf(filePath, format.c_str(), (i + startIdx + 1));
				this->loadFragment(filePath, particleType);
			}

			this->avgMass /= this->masses.size();
			cout << "Radius range is " << this->minRadius << " to " << this->maxRadius << endl;
			cout << "Mass range is " << this->minMass << " to " << this->maxMass << " with avg = " << this->avgMass << endl;
			cout << "SFR range is " << this->minSfr << " to " << this->maxSfr << endl;

			// Upload everything to the vbo.
			this->vbo.setVertexData(this->coordinates.data(), this->coordinates.size(), GL_STATIC_DRAW);
			this->vbo.setAttributeData(ExtraAttribute::Mass, this->masses.data(), 1, this->masses.size(), GL_STATIC_DRAW, 0);
			this->vbo.setAttributeData(ExtraAttribute::StarFormationRate, this->starFormationRates.data(), 1, this->starFormationRates.size(), GL_STATIC_DRAW, 0);
		
			//this->data.push_back(glm::vec4(0.0, 0.0, 0.0, 1.0));

			// Allocate the buffer.
			std::vector<InstanceData> tmpData(this->coordinates.size());
			bufferObj.allocate(tmpData, GL_DYNAMIC_DRAW);
		}
		
		//--------------------------------------------------------------
		void DataSet::clear()
		{
			this->paramListeners.clear();
			
			this->coordinates.clear();
			this->masses.clear();
			this->starFormationRates.clear();

			this->minRadius = std::numeric_limits<float>::max();
			this->maxRadius = std::numeric_limits<float>::min();

			this->minMass = std::numeric_limits<float>::max();
			this->maxMass = std::numeric_limits<float>::min();
			this->avgMass = 0.0f;

			this->minSfr = std::numeric_limits<float>::max();
			this->maxSfr = std::numeric_limits<float>::min();

			this->vbo.clear();
		}

		//--------------------------------------------------------------
		size_t DataSet::loadFragment(const std::string & filePath, const std::string & particleType)
		{
			static const int stride = 1;

			ofxHDF5File h5File;
			h5File.open(filePath, true);
			ofxHDF5GroupPtr h5Group = h5File.loadGroup(particleType);

			// Load the coordinate data and convert angles to radians.
			auto coordDataSet = h5Group->loadDataSet("Coordinates");
			int coordCount = coordDataSet->getDimensionSize(0) / stride;
			coordDataSet->setHyperslab(0, coordCount, stride);

			vector<glm::vec3> coordData(coordCount);
			coordDataSet->read(coordData.data());

			// Load the mass data.
			auto massDataSet = h5Group->loadDataSet("Masses");
			int massCount = massDataSet->getDimensionSize(0) / stride;
			massDataSet->setHyperslab(0, massCount, stride);

			vector<float> massData(massCount);
			massDataSet->read(massData.data());

			// Load the star formation rate data.
			auto sfrDataSet = h5Group->loadDataSet("StarFormationRate");
			int sfrCount = sfrDataSet->getDimensionSize(0) / stride;
			sfrDataSet->setHyperslab(0, sfrCount, stride);

			vector<float> sfrData(sfrCount);
			sfrDataSet->read(sfrData.data());

			// Add valid points to the data set.
			size_t total = 0;
			for (int i = 0; i < coordData.size(); ++i)
			{
				if (coordData[i].z > 0.0f)
				{
					this->coordinates.push_back(glm::vec3(ofDegToRad(coordData[i].x), ofDegToRad(coordData[i].y), coordData[i].z));
					this->minRadius = std::min(this->minRadius, coordData[i].z);
					this->maxRadius = std::max(this->maxRadius, coordData[i].z);

					this->masses.push_back(massData[i]);
					this->minMass = std::min(this->minMass, massData[i]);
					this->maxMass = std::max(this->maxMass, massData[i]);
					this->avgMass += massData[i];

					if (particleType == "PartType6")
					{
						this->starFormationRates.push_back(sfrData[i]);

						this->minSfr = std::min(this->minSfr, sfrData[i]);
						this->maxSfr = std::max(this->maxSfr, sfrData[i]);
					}
					else
					{
						// These are stars so just put in dummy data.
						this->starFormationRates.push_back(-1.0f);
					}

					++total;
				}
			}

			return total;
		}

		//--------------------------------------------------------------
		void DataSet::update(const glm::mat4 & worldTransform, const ofCamera & camera, const ofRectangle & viewport, SharedParams & sharedParams, bool updatePicking)
		{
			if (!this->parameters.renderModels) return;

			if (updatePicking)
			{
				this->pickingData.clear();
			}

			std::vector<InstanceData> data;

			const auto cameraModelView = camera.getModelViewMatrix();
			const auto cameraProjection = ofGetCurrentOrientationMatrix() * camera.getProjectionMatrix();
			
			float mappedMinMass = ofMap(sharedParams.model.clipMass, 0.0f, 1.0f, this->minMass, this->maxMass);

			float trackMinDist = std::numeric_limits<float>::max();

			for (int i = 0; i < this->coordinates.size(); ++i)
			{
				const auto & coords = this->coordinates[i];

				// Test that the point is within clipping mass.
				//if (i < 10) cout << "update() comparing mass " << this->masses[i] << " < " << mappedMinMass << endl;
				if (this->masses[i] < mappedMinMass)
				{
					//if (i == this->trackIdx) cout << "MASS " << this->masses[i] << " OOB " << mappedMinMass << endl;
					continue;
				}

				// Test that the point is within clipping bounds.
				if (this->mappedRadiusRange.x > coords.z ||
					this->mappedLongitudeRange.x > coords.x || coords.x > this->mappedLongitudeRange.y ||
					this->mappedLatitudeRange.x > coords.y || coords.y > this->mappedLatitudeRange.y)
				{
					//if (i == this->trackIdx) cout << "COORDS " << coords << " OOB" << endl;
					continue;
				}

				// Convert from spherical to Cartesian coordinates.
				const auto position = glm::vec4(coords.z * cos(coords.y) * cos(coords.x),
					coords.z * cos(coords.y) * sin(coords.x),
					coords.z * sin(coords.y),
					1.0f);

				// Test that the point is inside the visible frustum.
				const auto eyePos = cameraModelView * worldTransform * position;
				const auto camPos = cameraProjection * eyePos;
				const auto clipPos = camPos.xyz() / camPos.w;
				if (-1 > clipPos.x || clipPos.x > 1 ||
					-1 > clipPos.y || clipPos.y > 1 ||
					0 > clipPos.z || clipPos.z > 1)
				{
					//if (i == this->trackIdx) cout << "CLIPPOS " << clipPos << " OOB" << endl;
					continue;
				}

				// Test that the point is nearer than the fade distance.
				const float eyeDist = glm::length(eyePos.xyz());
				if (eyeDist > sharedParams.point.fadeFar)
				{
					continue;
				}
				
				// Passed all tests, add this instance!
				InstanceData instanceData;

				instanceData.dummy = (i == this->trackIdx ? 1.0 : 0.0);
				
				// Build and add transform matrix.
				const float modelSize = this->masses[i] * sharedParams.model.geoScale;
				auto scale = glm::vec3(modelSize);
				scale.y *= (1.0f - (((i % 53) / 53.0f) * sharedParams.model.squashRange));

				auto transform = glm::translate(worldTransform, position.xyz());
				transform = glm::scale(transform, scale);
				transform = glm::rotate(transform, i * 0.30302f, glm::normalize(glm::vec3(i % 11, i % 47, i % 23)));
				instanceData.transform = transform;

				// Calculate the size on screen (approximatively).
				const float attenuation = sharedParams.point.attenuation / eyeDist;
				const float screenSize = modelSize * attenuation * 0.33f; // this is nonsense, needs fixing.

				// Add the alpha value based on radius and distance from camera.
				if (this->mappedRadiusRange.y <= this->mappedRadiusRange.z)
				{
					float alpha = ofMap(position.z, this->mappedRadiusRange.y, this->mappedRadiusRange.z, 1.0f, 0.0f, true);
				
					if (eyeDist > sharedParams.point.fadeNear)
					{
						// Map distance from 0.0 to 1.0.
						alpha *= ofMap(eyeDist, sharedParams.point.fadeNear, sharedParams.point.fadeFar, 1.0f, 0.0f, true);
					}

					instanceData.alpha = alpha * sharedParams.model.alphaScale;
				}
				else
				{
					instanceData.alpha = 0.0f;
				}

				// Add the SFR although we don't use it yet.
				instanceData.starFormationRate = this->starFormationRates[i];

				// Add the density value based on screen size.
				//if (screenSize >= sharedParams.model.maxDensitySize)
				{
					instanceData.densityMod = 1;
				}
				//else
				{
					instanceData.alpha *= ofMap(screenSize, 1.0f, sharedParams.model.maxDensitySize, 0.5, 1);
				}

				data.push_back(instanceData);

				if (updatePicking)
				{
					const auto worldPos = (worldTransform * position).xyz();
					const auto screenPos = camera.worldToScreen(worldPos, viewport).xy();
					this->pickingData.push_back(std::make_pair(screenPos, worldPos));
				}

				if (this->findTrackPt)
				{
					const auto worldPos = (worldTransform * position).xyz();
					const auto screenPos = camera.worldToScreen(worldPos, viewport).xy();
					const auto currDist = glm::distance(screenPos, this->trackScreenPt);
					if (currDist < trackMinDist)
					{
						trackMinDist = currDist;
						this->trackIdx = i;
					}
				}
			}

			this->findTrackPt = false;

			if (data.empty())
			{
				this->modelCount = 0;
			}
			else
			{
				// Update the buffer.
				this->bufferObj.updateData(data);
				this->modelCount = data.size();
			}
		}
		
		//--------------------------------------------------------------
		glm::vec3 DataSet::getNearestScreenPoint(const glm::vec2 & pt) const
		{
			auto nearestDist = std::numeric_limits<float>::max();
			auto nearestPoint = glm::vec3(0.0f);
			for (auto & pair : this->pickingData) 
			{
				auto dist = glm::distance2(pair.first, pt);
				if (dist < nearestDist) {
					nearestDist = dist;
					nearestPoint = pair.second;
				}
			}
			return nearestPoint;
		}

		//--------------------------------------------------------------
		void DataSet::trackAtScreenPoint(const glm::vec2 & pt)
		{
			this->findTrackPt = true;
			this->trackScreenPt = pt;
		}

		//--------------------------------------------------------------
		void DataSet::drawPoints(ofShader & shader, SharedParams & sharedParams)
		{
			if (!this->parameters.renderPoints) return;

			float mappedClipMass = ofMap(sharedParams.model.clipMass, 0.0f, 1.0f, this->minMass, this->maxMass);

			shader.setUniform1f("uMaxMass", this->parameters.renderModels ? mappedClipMass : std::numeric_limits<float>::max());
			//shader.setUniform1f("uMaxMass", mappedClipMass);
			shader.setUniform1f("uCutRadius", this->mappedRadiusRange.x);
			shader.setUniform1f("uMinRadius", this->mappedRadiusRange.y);
			shader.setUniform1f("uMaxRadius", this->mappedRadiusRange.z);
			shader.setUniform1f("uMinLatitude", this->mappedLatitudeRange.x);
			shader.setUniform1f("uMaxLatitude", this->mappedLatitudeRange.y);
			shader.setUniform1f("uMinLongitude", this->mappedLongitudeRange.x);
			shader.setUniform1f("uMaxLongitude", this->mappedLongitudeRange.y);
			shader.setUniform1f("uMaxLongitude", this->mappedLongitudeRange.y);
			shader.setUniform1f("uBrightness", this->parameters.brightness);
			shader.setUniform4f("color1", this->parameters.color);
			shader.setUniform4f("color2", this->parameters.color2);
			shader.setUniform4f("color3", this->parameters.color3);
			shader.setUniform4f("color4", this->parameters.color4);

			//for (int i = 0; i < 10; ++i)
				//cout << "drawPoints() comparing mass " << this->masses[i] << " < " << mappedClipMass << endl;

			this->vbo.draw(GL_POINTS, 0, this->coordinates.size());
		}

		//--------------------------------------------------------------
		void DataSet::drawShells(ofShader & shader, SharedParams & sharedParams)
		{
			if (!this->parameters.renderShells) return;

			//shader.setUniform1f("uPointSize", ofMap(sharedParams.shell.size, 0.0f, 1.0f, this->mappedRadiusRange.y, this->mappedRadiusRange.z));
			shader.setUniform1f("uCutRadius", this->mappedRadiusRange.x);
			shader.setUniform1f("uMinRadius", this->mappedRadiusRange.y);
			shader.setUniform1f("uMaxRadius", this->mappedRadiusRange.z);
			shader.setUniform1f("uMinLatitude", this->mappedLatitudeRange.x);
			shader.setUniform1f("uMaxLatitude", this->mappedLatitudeRange.y);
			shader.setUniform1f("uMinLongitude", this->mappedLongitudeRange.x);
			shader.setUniform1f("uMaxLongitude", this->mappedLongitudeRange.y);
			shader.setUniform1f("uBrightness", this->parameters.brightness);
			shader.setUniform4f("globalColor", this->parameters.color);

			this->vbo.draw(GL_POINTS, 0, this->coordinates.size());
		}

		//--------------------------------------------------------------
		void DataSet::drawModels(ofShader & shader, SharedParams & sharedParams, ofVboMesh & mesh)
		{
			if (!this->parameters.renderModels) return;

			if (this->modelCount == 0) return;

			shader.setUniform1f("uBrightness", this->parameters.brightnessModels);
			shader.setUniform4f("color1", this->parameters.color);
			shader.setUniform4f("color2", this->parameters.color2);
			shader.setUniform4f("color3", this->parameters.color3);
			shader.setUniform4f("color4", this->parameters.color4);
			bufferObj.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
			ofSetColor(this->parameters.color.get());

			//cout << "Drawing " << this->modelCount << " models" << endl;
			if (sharedParams.model.useTestModel)
			{
				static ofVboMesh simpleMesh;
				if (simpleMesh.getNumVertices() == 0)
				{
					simpleMesh = ofVboMesh::sphere(1, 12, OF_PRIMITIVE_POINTS);
					for (int i = 0; i < simpleMesh.getNumVertices(); ++i)
					{
						simpleMesh.addColor(ofFloatColor::white);
					}
				}
				simpleMesh.drawInstanced(OF_MESH_POINTS, this->modelCount);
			}
			else
			{
				mesh.drawInstanced(OF_MESH_POINTS, this->modelCount);
			}
		}
	}
}
