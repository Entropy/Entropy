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
			cout << "Mass range is " << this->minMass << " to " << this->maxMass << " with avg = " << this->avgMass << endl;

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

			this->targetIndex = std::numeric_limits<int>::max();

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
		size_t DataSet::updateFilteredData(const glm::mat4 & worldTransform, const ofCamera & camera, SharedParams & params)
		{
			std::vector<InstanceData> data;

			const auto cameraModelView = camera.getModelViewMatrix();
			const auto cameraProjection = camera.getProjectionMatrix();
			
			int foundTargetIndex = std::numeric_limits<int>::max();
			float foundTargetDist = std::numeric_limits<float>::max();
			bool lockedTarget = false;
			
			for (int i = 0; i < this->coordinates.size(); ++i)
			{
				const auto & coords = this->coordinates[i];

				// Test that the point is within clipping bounds.
				if (this->mappedRadiusRange.x <= coords.z &&
					this->mappedLongitudeRange.x <= coords.x && coords.x <= this->mappedLongitudeRange.y &&
					this->mappedLatitudeRange.x <= coords.y && coords.y <= this->mappedLatitudeRange.y)
				{
					// Convert from spherical to Cartesian coordinates.
					const auto position = glm::vec4(coords.z * cos(coords.y) * cos(coords.x),
						coords.z * cos(coords.y) * sin(coords.x),
						coords.z * sin(coords.y),
						1.0f);

					// Test that the point is larger than the clipping size.
					const auto eyePos = cameraModelView * worldTransform * position;
					const float eyeDist = glm::length(eyePos.xyz());
					const float attenuation = params.point.attenuation / eyeDist;
					const float size = params.point.size * this->masses[i] * attenuation;
					if (size >= params.model.clipSize)
					{
						// Test that the point is inside the visible frustum.
						const auto camPos = cameraProjection * eyePos;
						const auto clipPos = camPos.xyz() / camPos.w;
						if (-1 <= clipPos.x && clipPos.x <= 1 &&
							-1 <= clipPos.y && clipPos.y <= 1 &&
							0 <= clipPos.z && clipPos.z <= 1)
						{
							// Passed all tests, build and add transform matrix!
							auto scale = glm::vec3(this->masses[i] * params.model.geoScale);
							scale.y *= (1.0f - (((i % 53) / 53.0f) * params.model.squashRange));

							auto transform = glm::translate(worldTransform, position.xyz());
							transform = glm::scale(transform, scale);
							transform = glm::rotate(transform, i * 0.30302f, glm::normalize(glm::vec3(i % 23, i % 43, i % 11)));
							// Hide the alpha value in the transform matrix.
							//transform[3][3] = ofMap(position.z, this->mappedRadiusRange.x, this->mappedRadiusRange.y, 1.0f, 0.0f, true);
							InstanceData instanceData;
							instanceData.transform = transform;
							if (this->mappedRadiusRange.y < this->mappedRadiusRange.z)
							{
								instanceData.alpha = ofMap(position.z, this->mappedRadiusRange.y, this->mappedRadiusRange.z, 1.0f, 0.0f, true) * params.model.alphaScale;
							}
							else
							{
								instanceData.alpha = 0.0f;
							}
							data.push_back(instanceData);

							if (i == this->targetIndex && params.target.lockDistance > 0.0f && params.target.lockDistance < eyeDist)
							{
								// Keep the current target locked in.
								// (This will set the foundTargetIndex to the current value and fail all subsequent tests)
								foundTargetDist = eyeDist;
								foundTargetIndex = i;
								lockedTarget = true;
								cout << "[DataSet] Keep it locked to " << this->targetIndex << endl;
							}
							else 
								if (!lockedTarget &&
								-0.5 <= clipPos.x && clipPos.x <= 0.5 &&
								-0.5 <= clipPos.y && clipPos.y <= 0.5 &&
								params.target.lockDistance < eyeDist && eyeDist < params.target.maxDistance && 
								foundTargetDist > eyeDist && this->masses[i] > params.target.minMass)
							{
								// Found new target.
								foundTargetDist = eyeDist;
								foundTargetIndex = i;
							}
						}
					}
				}
			}

			if (this->targetIndex != foundTargetIndex)
			{
				this->targetIndex = foundTargetIndex;
				cout << "[DataSet] Found new target with index = " << this->targetIndex << endl;
			}

			if (data.empty()) return 0;

			// Update the buffer.
			this->bufferObj.updateData(data);
			return data.size();
		}
		
		//--------------------------------------------------------------
		void DataSet::drawPoints(ofShader & shader)
		{
			if (!this->parameters.renderPoints) return;

			shader.setUniform1f("uCutRadius", this->mappedRadiusRange.x);
			shader.setUniform1f("uMinRadius", this->mappedRadiusRange.y);
			shader.setUniform1f("uMaxRadius", this->mappedRadiusRange.z);
			shader.setUniform1f("uMinLatitude", this->mappedLatitudeRange.x);
			shader.setUniform1f("uMaxLatitude", this->mappedLatitudeRange.y);
			shader.setUniform1f("uMinLongitude", this->mappedLongitudeRange.x);
			shader.setUniform1f("uMaxLongitude", this->mappedLongitudeRange.y);
			ofSetColor(this->parameters.color.get());

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
			ofSetColor(this->parameters.color.get());

			this->vbo.draw(GL_POINTS, 0, this->coordinates.size());
		}

		//--------------------------------------------------------------
		void DataSet::drawModels(ofShader & shader, const glm::mat4 & worldTransform, ofVboMesh & mesh, const ofCamera & camera, SharedParams & params)
		{
			if (!this->parameters.renderModels) return;
			
			if (params.model.clipDistance == 0.0f) return;

			auto clipCamera = camera;
			clipCamera.setFarClip(params.model.clipDistance);
			int count = this->updateFilteredData(worldTransform, clipCamera, params);
			
			if (count == 0) return;

			bufferObj.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
			ofSetColor(this->parameters.color.get());

			cout << "Drawing " << count << " models" << endl;
			//mesh.drawInstanced(OF_MESH_POINTS, count);

			static ofVboMesh simpleMesh = ofVboMesh::sphere(1, 12, OF_PRIMITIVE_POINTS);
			simpleMesh.drawInstanced(OF_MESH_POINTS, count);
		}

		//--------------------------------------------------------------
		int DataSet::getTargetIndex() const
		{
			return this->targetIndex;
		}

		//--------------------------------------------------------------
		glm::vec3 DataSet::getTargetPosition() const
		{
			if (this->targetIndex >= this->coordinates.size()) 
				return glm::vec3(0);

			const auto & coords = this->coordinates[this->targetIndex];
			const auto position = glm::vec3(coords.z * cos(coords.y) * cos(coords.x),
											coords.z * cos(coords.y) * sin(coords.x),
											coords.z * sin(coords.y));
			return position;
		}
		
		//--------------------------------------------------------------
		float DataSet::getTargetMass() const
		{
			if (this->targetIndex >= this->masses.size()) 
				return 0;

			return this->masses[this->targetIndex];
		}
	}
}
