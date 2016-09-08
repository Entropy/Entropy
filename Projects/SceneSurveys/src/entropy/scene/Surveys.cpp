#include "Surveys.h"

#include "entropy/surveys/GaussianMapTexture.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Surveys::Surveys()
			: Base()
		{}
		
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

			// Set ofParameterGroup names.
			this->parameters.setName("Surveys");
			this->backParameters.setName("Back");
			this->frontParameters.setName("Front");

			// Add extra parameters to the group (for serialization and timeline mappings).
			this->parameters.add(this->backParameters);
			this->parameters.add(this->frontParameters);
			this->parameters.add(this->dataSetBoss.parameters);
			this->parameters.add(this->dataSetDes.parameters);
			this->parameters.add(this->dataSetVizir.parameters);

			// Build the galaxy quad.
			this->galaxyQuad.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
			this->galaxyQuad.addVertex(glm::vec3(-1.0f, -1.0f, 0.0f));
			this->galaxyQuad.addVertex(glm::vec3(-1.0f,  1.0f, 0.0f));
			this->galaxyQuad.addVertex(glm::vec3( 1.0f, -1.0f, 0.0f));
			this->galaxyQuad.addVertex(glm::vec3( 1.0f,  1.0f, 0.0f));
			this->galaxyQuad.addTexCoord(glm::vec2(0.0f, 1.0f));
			this->galaxyQuad.addTexCoord(glm::vec2(0.0f, 0.0f));
			this->galaxyQuad.addTexCoord(glm::vec2(1.0f, 1.0f));
			this->galaxyQuad.addTexCoord(glm::vec2(1.0f, 0.0f));

			// Build the texture.
			//entropy::survey::CreateGaussianMapTexture(texture, 32, GL_TEXTURE_2D);
			const auto filePath = this->getAssetsPath("images/sprites.png");
			//const auto filePath = this->getAssetsPath("images/Gaia_star_density_image_log.png");
			ofPixels pixels;
			ofLoadImage(pixels, filePath);
			if (!pixels.isAllocated())
			{
				ofLogError(__FUNCTION__) << "Could not load file at path " << filePath;
			}

			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->texture.enableMipmap();
				this->texture.loadData(pixels);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			// Load the shader.
			this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
			this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
			this->spriteShader.bindAttribute(surveys::ExtraAttribute::Mass, "mass");
			this->spriteShader.bindAttribute(surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
			this->spriteShader.bindDefaults();
			this->spriteShader.linkProgram();
		}

		//--------------------------------------------------------------
		void Surveys::clear()
		{
			// Clear the data.
			this->dataSetBoss.clear();
			this->dataSetDes.clear();
			this->dataSetVizir.clear();

			// Clear the texture.
			texture.clear();
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
				this->cameras[render::Layout::Back]->reset();
			}
		}
		
		//--------------------------------------------------------------
		void Surveys::drawBackWorld()
		{
			this->drawDataSet(this->backParameters);
		}

		//--------------------------------------------------------------
		void Surveys::drawFrontWorld()
		{
			this->drawDataSet(this->frontParameters);
		}

		//--------------------------------------------------------------
		void Surveys::drawDataSet(LayoutParameters & parameters)
		{
			// Draw the galaxy in the center.
			ofPushMatrix();
			{
				ofScale(this->parameters.galaxy.scale);
				ofRotateX(this->parameters.galaxy.orientation.get().x);
				ofRotateY(this->parameters.galaxy.orientation.get().y);
				ofRotateZ(this->parameters.galaxy.orientation.get().z);

				ofPushStyle();
				{
					ofSetColor(255, this->parameters.galaxy.alpha * 255);

					this->texture.bind();
					this->galaxyQuad.draw();
					this->texture.unbind();
				}
				ofPopStyle();
			}
			ofPopMatrix();

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
				ofxPreset::Gui::AddGroup(this->parameters.galaxy, settings);
				
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