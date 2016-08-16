#include "DataSet.h"

#include "ofxHDF5.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace darkness
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
		void DataSet::setup(const std::string & name, const std::string & format, size_t startIdx, size_t count)
		{
			this->parameters.setName(name);

			this->clear();

			// Load the data one fragment at a time.
			std::vector<glm::vec3> vertices;
			std::vector<float> masses;
			this->counts.resize(count);
			for (int i = 0; i < count; ++i)
			{
				char filename[256];
				sprintf(filename, format.c_str(), (i + startIdx + 1));
				const auto filePath = GetCurrentSceneAssetsPath(filename);

				this->counts[i] = this->loadFragment(filePath, vertices, masses);
			}

			// Upload everything to the vbo.
			this->vbo.setVertexData(vertices.data(), vertices.size(), GL_STATIC_DRAW);
			this->vbo.setAttributeData(MASS_ATTRIBUTE, masses.data(), 1, masses.size(), GL_STATIC_DRAW, 0);
		}
		
		//--------------------------------------------------------------
		void DataSet::clear()
		{
			this->fragments.clear();
			this->counts.clear();

			for (int i = 0; i < MAX_FRAGMENTS; ++i)
			{
				this->enabled[i] = false;
			}

			this->vbo.clear();
			this->vboDirty = false;
		}

		//--------------------------------------------------------------
		size_t DataSet::loadFragment(const string & filePath, vector<glm::vec3> & vertices, vector<float> & masses)
		{
			static const int stride = 1;

			ofxHDF5File h5File;
			h5File.open(filePath, true);
			ofxHDF5GroupPtr h5Group = h5File.loadGroup("PartType6");

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
			std::vector<Point> points(coordsData.size());
			for (int i = 0; i < coordsData.size(); ++i)
			{
				points[i] = { coordsData[i].x, coordsData[i].y, coordsData[i].z, massesData[i] };
			}

			// Sort the data points by radius.
			std::sort(points.begin(), points.end(), [](const Point & a, const Point & b) -> bool
			{
				return (a.radius > b.radius);
			});

			// Convert the position data to Cartesian coordinates and store all data in the passed vectors.
			int startIdx = vertices.size();
			vertices.resize(startIdx + points.size());
			masses.resize(startIdx + massesData.size());
			for (int i = 0; i < points.size(); ++i)
			{
				int idx = startIdx + i;
				vertices[idx].x = points[i].radius * cos(ofDegToRad(points[i].latitude)) * cos(ofDegToRad(points[i].longitude));
				vertices[idx].y = points[i].radius * cos(ofDegToRad(points[i].latitude)) * sin(ofDegToRad(points[i].longitude));
				vertices[idx].z = points[i].radius * sin(ofDegToRad(points[i].latitude));
				masses[idx] = massesData[i];
			}

			return points.size();
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

			int total = 0;
			for (int i = 0; i < this->parameters.fragments; ++i)
			{
				total += this->counts[i];
			}

			this->vbo.draw(GL_POINTS, 0, total);
		}

		//--------------------------------------------------------------
		void DataSet::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ImGui::CollapsingHeader(this->parameters.getName().c_str(), nullptr, true, true))
			{
				ofxPreset::Gui::AddParameter(this->parameters.fragments);					
				ofxPreset::Gui::AddRange("Distance", this->parameters.minDistance, this->parameters.maxDistance);
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