#include "Darkness.h"

#include "ofxHDF5.h"
#include "entropy/darkness/GaussianMapTexture.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Darkness::Darkness()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Darkness::~Darkness()
		{

		}

		//--------------------------------------------------------------
		void Darkness::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			// Load initial data.
			for (int i = 0; i < 10; i++)
			{
				const auto filename = this->getAssetsPath("particles/boss_fragment-batch-" + ofToString(i + 1) + "of10.hdf5");// "sample_contig.hdf5";
				this->loadData(filename, this->vboBoss);
			}
			for (int i = 0; i < 20; i++)
			{
				const auto filename = this->getAssetsPath("particles/des_fragment-batch-" + ofToString(i + 1) + "of20.hdf5");// "sample_contig.hdf5";
				this->loadData(filename, this->vboDes);
			}

			// Build the texture.
			entropy::darkness::CreateGaussianMapTexture(texture, 32, GL_TEXTURE_2D);

			// Load the shader.
			this->shader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/render.vert");
			this->shader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/render.frag");
			this->shader.bindAttribute(MASS_ATTRIBUTE, "mass");
			this->shader.bindDefaults();
			this->shader.linkProgram();
		}
		
		//--------------------------------------------------------------
		void Darkness::exit()
		{

		}

		//--------------------------------------------------------------
		void Darkness::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void Darkness::update(double & dt)
		{

		}

		//--------------------------------------------------------------
		void Darkness::drawBack()
		{

		}
		
		//--------------------------------------------------------------
		void Darkness::drawWorld()
		{
			glEnable(GL_POINT_SMOOTH);
			glPointSize(this->parameters.pointSize);

			ofPushMatrix();
			ofScale(this->parameters.scale);
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();
					
				if (this->parameters.useSprites) {
					this->shader.begin();
					this->shader.setUniformTexture("texx", texture, 1);
					this->shader.setUniform1f("pointSize", this->parameters.pointSize);
					ofEnablePointSprites();
				}
				else {
					glPointSize(this->parameters.pointSize);
				}

				ofSetColor(ofColor::red);
				this->vboBoss.draw(OF_MESH_POINTS);

				ofSetColor(ofColor::blue);
				this->vboDes.draw(OF_MESH_POINTS);

				if (this->parameters.useSprites) {
					ofDisablePointSprites();
					this->shader.end();
				}
				else {
					glPointSize(1.0f);
				}

				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}
			ofPopMatrix();
		}

		//--------------------------------------------------------------
		void Darkness::drawFront()
		{

		}

		//--------------------------------------------------------------
		void Darkness::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				ofxPreset::Gui::AddParameter(this->parameters.scale);
				ofxPreset::Gui::AddParameter(this->parameters.pointSize);
				ofxPreset::Gui::AddParameter(this->parameters.useSprites);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Darkness::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		void Darkness::deserialize(const nlohmann::json & json)
		{

		}

		//--------------------------------------------------------------
		void Darkness::loadData(const string & filePath, ofVboMesh & vboMesh)
		{
			const int stride = 1;

			ofxHDF5File h5File;
			h5File.open(filePath, true);
			ofxHDF5GroupPtr h5Group = h5File.loadGroup("PartType6");

			ofxHDF5DataSetPtr coordsDataSet = h5Group->loadDataSet("Coordinates");
			int coordsCount = coordsDataSet->getDimensionSize(0) / stride;
			coordsDataSet->setHyperslab(0, coordsCount, stride);
			//
			vector<Coordinate> coordsData(coordsCount);
			coordsDataSet->read(coordsData.data());

			// Load the mass data.
			ofxHDF5DataSetPtr massesDataSet = h5Group->loadDataSet("Masses");
			int massesCount = massesDataSet->getDimensionSize(0) / stride;
			massesDataSet->setHyperslab(0, massesCount, stride);

			vector<float> massesData(massesCount);
			massesDataSet->read(massesData.data());

			// Convert the position data to Cartesian coordinates.
			vector<glm::vec3> vertices(coordsCount);
			for (int i = 0; i < vertices.size(); ++i) 
			{
				vertices[i].x = coordsData[i].radius * cos(ofDegToRad(coordsData[i].latitude)) * cos(ofDegToRad(coordsData[i].longitude));
				vertices[i].y = coordsData[i].radius * cos(ofDegToRad(coordsData[i].latitude)) * sin(ofDegToRad(coordsData[i].longitude));
				vertices[i].z = coordsData[i].radius * sin(ofDegToRad(coordsData[i].latitude));
			}

			// Upload everything to the VBO.
			//vboMesh.clear();
			vboMesh.addVertices(vertices);
			vboMesh.getVbo().setAttributeData(MASS_ATTRIBUTE, massesData.data(), 1, massesData.size(), GL_STATIC_DRAW, 0);
		}
	}
}