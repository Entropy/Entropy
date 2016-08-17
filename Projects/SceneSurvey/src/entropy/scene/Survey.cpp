#include "Survey.h"

#include "entropy/survey/GaussianMapTexture.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Survey::Survey()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Survey::~Survey()
		{

		}

		//--------------------------------------------------------------
		void Survey::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			// Load data.
			this->dataSetBoss.setup("BOSS", "particles/boss_fragment-batch-%iof10.hdf5", 0, 10);
			this->dataSetDes.setup("DES", "particles/des_fragment-batch-%iof20.hdf5", 0, 20);

			// Build the texture.
			entropy::survey::CreateGaussianMapTexture(texture, 32, GL_TEXTURE_2D);

			// Load the shader.
			this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
			this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
			this->spriteShader.bindAttribute(entropy::survey::DataSet::MASS_ATTRIBUTE, "mass");
			this->spriteShader.bindDefaults();
			this->spriteShader.linkProgram();
		}
		
		//--------------------------------------------------------------
		void Survey::exit()
		{

		}

		//--------------------------------------------------------------
		void Survey::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void Survey::update(double & dt)
		{

		}

		//--------------------------------------------------------------
		void Survey::drawBack()
		{

		}
		
		//--------------------------------------------------------------
		void Survey::drawWorld()
		{
			glEnable(GL_POINT_SMOOTH);
			glPointSize(this->parameters.render.pointSize);

			ofPushMatrix();
			ofScale(this->parameters.render.scale);
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();
					
				if (this->parameters.render.useSprites) {
					this->spriteShader.begin();
					this->spriteShader.setUniformTexture("uTex0", texture, 1);
					this->spriteShader.setUniform1f("uPointSize", this->parameters.render.pointSize);
					ofEnablePointSprites();
				}
				else {
					glPointSize(this->parameters.render.pointSize);
				}

				//if (this->parameters.boss.fragments > 0)
				//{
				//	int total = 0;
				//	for (int i = 0; i < this->parameters.boss.fragments; ++i)
				//	{
				//		total += this->dataSetBoss.counts[i];
				//	}
				//	ofSetColor(ofColor::red);
				//	this->dataSetBoss.vboMesh.getVbo().draw(GL_POINTS, 0, total);
				//}

				//if (this->parameters.des.fragments > 0)
				//{
				//	int total = 0;
				//	for (int i = 0; i < this->parameters.des.fragments; ++i)
				//	{
				//		total += this->dataSetDes.counts[i];
				//	}
				//	ofSetColor(ofColor::blue);
				//	this->dataSetDes.vboMesh.getVbo().draw(GL_POINTS, 0, total);
				//}
				this->dataSetBoss.draw();
				this->dataSetDes.draw();

				if (this->parameters.render.useSprites) {
					ofDisablePointSprites();
					this->spriteShader.end();
				}
				else {
					glPointSize(1.0f);
				}

				ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			}
			ofPopMatrix();
		}

		//--------------------------------------------------------------
		void Survey::drawFront()
		{

		}

		//--------------------------------------------------------------
		void Survey::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				ofxPreset::Gui::AddGroup(this->parameters.render, settings);

				this->dataSetBoss.gui(settings);
				this->dataSetDes.gui(settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Survey::serialize(nlohmann::json & json)
		{
			this->dataSetBoss.serialize(json);
			this->dataSetDes.serialize(json);
		}
		
		//--------------------------------------------------------------
		void Survey::deserialize(const nlohmann::json & json)
		{
			this->dataSetBoss.deserialize(json);
			this->dataSetDes.deserialize(json);
		}
	}
}