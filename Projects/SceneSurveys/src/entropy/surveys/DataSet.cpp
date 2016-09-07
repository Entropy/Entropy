#include "DataSet.h"

#include "ofxHDF5.h"

#include "entropy/Helpers.h"

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

			// Load the data one fragment at a time.
			for (int i = 0; i < count; ++i)
			{
				char filePath[512];
				sprintf(filePath, format.c_str(), (i + startIdx + 1));
				this->loadFragment(filePath, particleType);
			}

			// Upload everything to the vbo.
			this->vbo.setVertexData(this->coordinates.data(), this->coordinates.size(), GL_STATIC_DRAW);
			this->vbo.setAttributeData(ExtraAttribute::Mass, this->masses.data(), 1, this->masses.size(), GL_STATIC_DRAW, 0);
			this->vbo.setAttributeData(ExtraAttribute::StarFormationRate, this->starFormationRates.data(), 1, this->starFormationRates.size(), GL_STATIC_DRAW, 0);
		}
		
		//--------------------------------------------------------------
		void DataSet::clear()
		{
			this->coordinates.clear();
			this->masses.clear();
			this->starFormationRates.clear();

			this->minRadius = std::numeric_limits<float>::max();
			this->maxRadius = std::numeric_limits<float>::min();

			this->vbo.clear();
		}

		//--------------------------------------------------------------
		size_t DataSet::loadFragment(const std::string & filePath, const std::string & particleType)
		{
			static const int stride = 1;

			ofxHDF5File h5File;
			h5File.open(filePath, true);
			ofxHDF5GroupPtr h5Group = h5File.loadGroup(particleType);

			size_t total = 0;

			// Load the coordinate data and convert angles to radians.
			{
				auto dataSet = h5Group->loadDataSet("Coordinates");
				int count = dataSet->getDimensionSize(0) / stride;
				dataSet->setHyperslab(0, count, stride);

				vector<glm::vec3> coordsData(count);
				dataSet->read(coordsData.data());

				for (int i = 0; i < coordsData.size(); ++i)
				{
					this->coordinates.push_back(glm::vec3(ofDegToRad(coordsData[i].x), ofDegToRad(coordsData[i].y), coordsData[i].z));
					this->minRadius = std::min(this->minRadius, coordsData[i].z);
					this->maxRadius = std::max(this->maxRadius, coordsData[i].z);
				}

				total = count;
			}

			// Load the mass data in place.
			{
				auto dataSet = h5Group->loadDataSet("Masses");
				int count = dataSet->getDimensionSize(0) / stride;
				dataSet->setHyperslab(0, count, stride);

				int firstIdx = this->masses.size();
				this->masses.resize(firstIdx + count);
				dataSet->read(&this->masses[firstIdx]);
			}

			if (particleType == "PartType6")
			{
				// Load the star formation rate data in place.
				auto dataSet = h5Group->loadDataSet("StarFormationRate");
				int count = dataSet->getDimensionSize(0) / stride;
				dataSet->setHyperslab(0, count, stride);

				int firstIdx = this->starFormationRates.size();
				this->starFormationRates.resize(firstIdx + count);
				dataSet->read(&this->starFormationRates[firstIdx]);
			}
			else
			{
				// These aren't galaxies so just fill with dummy data.
				int firstIdx = this->starFormationRates.size();
				this->starFormationRates.resize(firstIdx + total);

				for (int i = firstIdx; i < this->starFormationRates.size(); ++i)
				{
					this->starFormationRates[i] = -1.0f;
				}
			}

			return total;
		}
		
		//--------------------------------------------------------------
		void DataSet::draw(ofShader & shader)
		{
			static const auto kLatitudeMin = -HALF_PI;
			static const auto kLatitudeMax = HALF_PI;
			static const auto kLongitudeMin = 0;
			static const auto kLongitudeMax = TWO_PI;

			shader.setUniform1f("uMinRadius", ofMap(parameters.minRadius, 0.0f, 1.0f, this->minRadius, this->maxRadius));
			shader.setUniform1f("uMaxRadius", ofMap(parameters.maxRadius, 0.0f, 1.0f, this->minRadius, this->maxRadius));
			shader.setUniform1f("uMinLatitude", ofMap(parameters.minLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
			shader.setUniform1f("uMaxLatitude", ofMap(parameters.maxLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
			shader.setUniform1f("uMinLongitude", ofMap(parameters.minLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
			shader.setUniform1f("uMaxLongitude", ofMap(parameters.maxLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
			
			ofSetColor(this->parameters.color.get());

			this->vbo.draw(GL_POINTS, 0, this->coordinates.size());
		}

		//--------------------------------------------------------------
		void DataSet::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ImGui::CollapsingHeader(this->parameters.getName().c_str(), nullptr, true, true))
			{
				ofxPreset::Gui::AddRange("Radius", this->parameters.minRadius, this->parameters.maxRadius);
				ofxPreset::Gui::AddRange("Latitude", this->parameters.minLatitude, this->parameters.maxLatitude);
				ofxPreset::Gui::AddRange("Longitude", this->parameters.minLongitude, this->parameters.maxLongitude);
				ofxPreset::Gui::AddParameter(this->parameters.color);
			}
		}

		//--------------------------------------------------------------
		void DataSet::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->parameters);
		}

		//--------------------------------------------------------------
		void DataSet::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->parameters);
		}
	}
}