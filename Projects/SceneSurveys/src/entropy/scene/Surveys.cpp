#include "Surveys.h"

#include "entropy/surveys/GaussianMapTexture.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Surveys::Surveys()
			: Base()
		{
			this->parameters.setName("Surveys");
		}
		
		//--------------------------------------------------------------
		Surveys::~Surveys()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Surveys::init()
		{
			// Load the data.
			this->dataSetBoss.setup("BOSS", this->getAssetsPath("particles/boss_fragment-batch-%iof10.hdf5"), 0, 10, "PartType6");
			this->dataSetDes.setup("DES", this->getAssetsPath("particles/des_fragment-batch-%iof20.hdf5"), 0, 20, "PartType6");
			this->dataSetVizir.setup("ViziR", this->getAssetsPath("particles/Hipparchos-Tycho-stars-fromViziR.hdf5"), 0, 1, "PartType4");

			// Init the sphere.
			this->loadTextureImage(this->getAssetsPath("images/The_Milky_Way.png"), this->sphereTexture);

			auto shaderSettings = ofShader::Settings();
			shaderSettings.bindDefaults = true;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/galaxy.vert";
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/galaxy.frag";
			this->sphereShader.setup(shaderSettings);

			// Build the texture.
			//entropy::survey::CreateGaussianMapTexture(this->spriteTexture, 32, GL_TEXTURE_2D);
			this->loadTextureImage(this->getAssetsPath("images/sprites.png"), this->spriteTexture);

			// Load the shader.
			this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
			this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
			this->spriteShader.bindAttribute(surveys::ExtraAttribute::Mass, "mass");
			this->spriteShader.bindAttribute(surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
			this->spriteShader.bindDefaults();
			this->spriteShader.linkProgram();

			// Init parameters.
			this->backParameters.setName("Back");
			this->frontParameters.setName("Front");

			this->parameters.add(this->backParameters);
			this->parameters.add(this->frontParameters);
			this->parameters.add(this->dataSetBoss.parameters);
			this->parameters.add(this->dataSetDes.parameters);
			this->parameters.add(this->dataSetVizir.parameters);
			this->parameters.add(this->sphereGeom.parameters);
		}

		//--------------------------------------------------------------
		void Surveys::clear()
		{
			this->spriteShader.unload();

			this->dataSetBoss.clear();
			this->dataSetDes.clear();
			this->dataSetVizir.clear();

			this->spriteTexture.clear();

			this->sphereTexture.clear();
			this->sphereGeom.clear();
		}

		//--------------------------------------------------------------
		void Surveys::setup()
		{}
		
		//--------------------------------------------------------------
		void Surveys::exit()
		{}

		//--------------------------------------------------------------
		void Surveys::timelineBangFired(ofxTLBangEventArgs & args)
		{
			static const string kResetFlag = "reset";
			if (args.flag.compare(0, kResetFlag.size(), kResetFlag) == 0)
			{
				// Reset the camera tumble.
				this->cameras[render::Layout::Back]->reset(false);
			}
		}
		
		//--------------------------------------------------------------
		void Surveys::drawBackWorld()
		{
			this->drawDataSet(this->backParameters);

			// Draw the galaxy.
			this->sphereShader.begin();
			{
				this->sphereShader.setUniformMatrix4f("uNormalMatrix", ofGetCurrentNormalMatrix());
				//this->sphereShader.setUniform1f("uRadius", this->sphereGeom.radius);
				this->sphereShader.setUniformTexture("uTex0", this->sphereTexture, 1);
				this->sphereShader.setUniform1f("uAlphaBase", this->sphereGeom.alpha);

				this->sphereGeom.draw();
			}
			this->sphereShader.end();

			//ofSetColor(ofColor::green);
			//auto & mesh = this->sphereGeom.getMesh();
			//for (int i = 0; i < mesh.getNumVertices(); ++i)
			//{
			//	auto vert = mesh.getVertex(i);
			//	auto norm = mesh.getNormal(i);
			//	ofDrawLine(vert, vert + norm * 100);
			//}
		}

		//--------------------------------------------------------------
		void Surveys::drawFrontWorld()
		{
			this->drawDataSet(this->frontParameters);
		}

		//--------------------------------------------------------------
		void Surveys::drawDataSet(LayoutParameters & parameters)
		{
			ofPushMatrix();
			ofScale(parameters.scale);
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				this->spriteShader.begin();
				this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
				this->spriteShader.setUniform1f("uPointSize", parameters.pointSize);
				ofEnablePointSprites();
				{
					if (parameters.renderBoss)
					{
						this->dataSetBoss.draw(this->spriteShader);
					}
					if (parameters.renderDes)
					{
						this->dataSetDes.draw(this->spriteShader);
					}
					if (parameters.renderVizir)
					{
						this->dataSetVizir.draw(this->spriteShader);
					}
				}
				ofDisablePointSprites();
				this->spriteShader.end();
				
				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}
			ofPopMatrix();
		}

		//--------------------------------------------------------------
		void Surveys::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				if (ofxPreset::Gui::BeginTree(this->sphereGeom.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->sphereGeom.enabled);
					static const vector<string> blendLabels{ "Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen" };
					ofxPreset::Gui::AddRadio(this->sphereGeom.blendMode, blendLabels, 3);
					ofxPreset::Gui::AddParameter(this->sphereGeom.depthTest);
					static const vector<string> cullLabels{ "None", "Back", "Front" };
					ofxPreset::Gui::AddRadio(this->sphereGeom.cullFace, cullLabels, 3);
					ofxPreset::Gui::AddParameter(this->sphereGeom.color);
					ofxPreset::Gui::AddParameter(this->sphereGeom.alpha);
					ofxPreset::Gui::AddParameter(this->sphereGeom.radius);
					ofxPreset::Gui::AddParameter(this->sphereGeom.resolution);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcHorz);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcVert);
					ofxPreset::Gui::AddParameter(this->sphereGeom.orientation);

					ofxPreset::Gui::EndTree(settings);
				}
				
				this->dataSetBoss.gui(settings);
				this->dataSetDes.gui(settings);
				this->dataSetVizir.gui(settings);

				ofxPreset::Gui::AddGroup(this->backParameters, settings);
				ofxPreset::Gui::AddGroup(this->frontParameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}
	}
}