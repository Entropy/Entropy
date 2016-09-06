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
				this->loadFragment(filePath, particleType, this->points);
			}

			// Sort the data points by radius.
			std::sort(this->points.begin(), this->points.end(), [](const glm::vec4 & a, const glm::vec4 & b) -> bool
			{
				return (a.z < b.z);
			});

			// Upload everything to the vbo.
			this->vbo.setVertexData((float *)(this->points.data()), 4, this->points.size(), GL_STATIC_DRAW, sizeof(glm::vec4));
		}
		
		//--------------------------------------------------------------
		void DataSet::clear()
		{
			this->points.clear();

			this->vbo.clear();
			this->vboDirty = false;
		}

		//--------------------------------------------------------------
		size_t DataSet::loadFragment(const std::string & filePath, const std::string & particleType, std::vector<glm::vec4> & points)
		{
			static const int stride = 1;

			ofxHDF5File h5File;
			h5File.open(filePath, true);
			ofxHDF5GroupPtr h5Group = h5File.loadGroup(particleType);

			// Load the coordinate data.
			auto coordsDataSet = h5Group->loadDataSet("Coordinates");
			int coordsCount = coordsDataSet->getDimensionSize(0) / stride;
			coordsDataSet->setHyperslab(0, coordsCount, stride);

			vector<glm::vec3> coordsData(coordsCount);
			coordsDataSet->read(coordsData.data());

			// Load the mass data.
			auto massesDataSet = h5Group->loadDataSet("Masses");
			int massesCount = massesDataSet->getDimensionSize(0) / stride;
			massesDataSet->setHyperslab(0, massesCount, stride);

			vector<float> massesData(massesCount);
			massesDataSet->read(massesData.data());

			// Combine the data into a single struct.
			// x == longitude
			// y == latitude
			// z == radius
			// w == mass
			for (int i = 0; i < coordsData.size(); ++i)
			{
				points.push_back(glm::vec4(ofDegToRad(coordsData[i].x), ofDegToRad(coordsData[i].y), coordsData[i].z, massesData[i]));
			}

			return coordsData.size();
		}

		//--------------------------------------------------------------
		void DataSet::update()
		{
			if (this->vboDirty)
			{
				this->vboDirty = false;
			}
		}
		
		//--------------------------------------------------------------
		void DataSet::draw()
		{
			ofSetColor(this->parameters.color.get());

			int startIdx = ofMap(this->parameters.minRadius, 0.0f, 1.0f, 0, this->points.size(), true);
			int endIdx = ofMap(this->parameters.maxRadius, 0.0f, 1.0f, 0, this->points.size(), true);

			this->vbo.draw(GL_POINTS, startIdx, endIdx - startIdx + 1);
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