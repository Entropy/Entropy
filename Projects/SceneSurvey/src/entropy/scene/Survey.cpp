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
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Survey::init()
		{
			// Load the data.
			this->dataSetBoss.setup("BOSS", this->getAssetsPath("particles/boss_fragment-batch-%iof10.hdf5"), 0, 10);
			this->dataSetDes.setup("DES", this->getAssetsPath("particles/des_fragment-batch-%iof20.hdf5"), 0, 20);
			
			// Set ofParameterGroup names.
			this->parameters.setName("Survey");
			this->backParameters.setName("Back");
			this->frontParameters.setName("Front");

			// Add extra parameters to the group (for serialization and timeline mappings).
			this->parameters.add(this->backParameters);
			this->parameters.add(this->frontParameters);
			this->parameters.add(this->dataSetBoss.parameters);
			this->parameters.add(this->dataSetDes.parameters);

			// Build the texture.
			entropy::survey::CreateGaussianMapTexture(texture, 32, GL_TEXTURE_2D);

			// Load the shader.
			this->spriteShader.load("shaders/sprite");
		}

		//--------------------------------------------------------------
		void Survey::clear()
		{
			// Clear the data.
			this->dataSetBoss.clear();
			this->dataSetDes.clear();

			// Clear the texture.
			texture.clear();
		}

		//--------------------------------------------------------------
		void Survey::setup()
		{}
		
		//--------------------------------------------------------------
		void Survey::exit()
		{}
		
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
			ofPushMatrix();
			ofScale(parameters.scale);
			{
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				this->spriteShader.begin();
				this->spriteShader.setUniformTexture("uTex0", texture, 1);
				this->spriteShader.setUniform1f("uPointSize", parameters.pointSize);
				ofEnablePointSprites();
				{
					static const auto kLatitudeMin = -HALF_PI;
					static const auto kLatitudeMax = HALF_PI;
					static const auto kLongitudeMin = 0;
					static const auto kLongitudeMax = TWO_PI;

					if (parameters.renderBoss)
					{
						this->spriteShader.setUniform1f("uMinLatitude", ofMap(this->dataSetBoss.parameters.minLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
						this->spriteShader.setUniform1f("uMaxLatitude", ofMap(this->dataSetBoss.parameters.maxLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
						this->spriteShader.setUniform1f("uMinLongitude", ofMap(this->dataSetBoss.parameters.minLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
						this->spriteShader.setUniform1f("uMaxLongitude", ofMap(this->dataSetBoss.parameters.maxLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
						this->dataSetBoss.draw();
					}
					if (parameters.renderDes)
					{
						this->spriteShader.setUniform1f("uMinLatitude", ofMap(this->dataSetDes.parameters.minLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
						this->spriteShader.setUniform1f("uMaxLatitude", ofMap(this->dataSetDes.parameters.maxLatitude, 0.0f, 1.0f, kLatitudeMin, kLatitudeMax));
						this->spriteShader.setUniform1f("uMinLongitude", ofMap(this->dataSetDes.parameters.minLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
						this->spriteShader.setUniform1f("uMaxLongitude", ofMap(this->dataSetDes.parameters.maxLongitude, 0.0f, 1.0f, kLongitudeMin, kLongitudeMax));
						this->dataSetDes.draw();
					}
				}
				ofDisablePointSprites();
				this->spriteShader.end();
				
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