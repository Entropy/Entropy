#include "Survey.h"

#include "entropy/survey/GaussianMapTexture.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Survey::Survey()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Survey::~Survey()
		{}

		//--------------------------------------------------------------
		void Survey::setup()
		{
			// Load data.
			this->dataSetBoss.setup("BOSS", "particles/boss_fragment-batch-%iof10.hdf5", 0, 10);
			this->dataSetDes.setup("DES", "particles/des_fragment-batch-%iof20.hdf5", 0, 20);

			// Set ofParameterGroup names.
			this->parameters.setName("Survey");
			this->backParameters.setName("Back");
			this->frontParameters.setName("Front");
			
			// Add extra parameters to the group (for serialization and ofxTimeline mappings).
			this->parameters.add(this->backParameters);
			this->parameters.add(this->frontParameters);
			this->parameters.add(this->dataSetBoss.parameters);
			this->parameters.add(this->dataSetDes.parameters);

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
			this->dataSetBoss.clear();
			this->dataSetDes.clear();

			texture.clear();
		}
		
		//--------------------------------------------------------------
		void Survey::drawBackWorld()
		{
			this->drawDataSet(this->backParameters);
		}

		//--------------------------------------------------------------
		void Survey::drawFrontWorld()
		{
			this->drawDataSet(this->frontParameters);
		}

		//--------------------------------------------------------------
		void Survey::drawDataSet(LayoutParameters & parameters)
		{
			glEnable(GL_POINT_SMOOTH);
			glPointSize(parameters.pointSize);

			ofPushMatrix();
			ofScale(parameters.scale);
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				if (parameters.useSprites) {
					this->spriteShader.begin();
					this->spriteShader.setUniformTexture("uTex0", texture, 1);
					this->spriteShader.setUniform1f("uPointSize", parameters.pointSize);
					ofEnablePointSprites();
				}
				else {
					glPointSize(parameters.pointSize);
				}

				if (parameters.renderBoss)
				{
					this->dataSetBoss.draw();
				}
				if (parameters.renderDes)
				{
					this->dataSetDes.draw();
				}

				if (parameters.useSprites) {
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
		void Survey::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				this->dataSetBoss.gui(settings);
				this->dataSetDes.gui(settings);

				ofxPreset::Gui::AddGroup(this->backParameters, settings);
				ofxPreset::Gui::AddGroup(this->frontParameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}
	}
}