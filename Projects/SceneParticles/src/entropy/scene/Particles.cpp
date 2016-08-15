#include "Particles.h"
#include "glm/gtc/random.hpp"

#include "entropy/Helpers.h"

//#define _TEAPOT

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		const float Particles::HALF_DIM = 400.f;

		//--------------------------------------------------------------
		Particles::Particles()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}

		//--------------------------------------------------------------
		Particles::~Particles()
		{

		}

		//--------------------------------------------------------------
		// Set up your crap here!
		void Particles::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			environment = nm::Environment::Ptr(new nm::Environment(glm::vec3(-HALF_DIM), glm::vec3(HALF_DIM)));
			particleSystem.init(environment);
			photons.init(environment);

#ifdef _TEAPOT
			ofVboMesh mesh;
			ofxObjLoader::load("teapot.obj", mesh);
			for (auto& v : mesh.getVertices())
			{
				v *= 80.f;
				v.y -= 80.f;
				particleSystem.addParticle(v);
			}
#else
			for (unsigned i = 0; i < 4000; ++i)
			{
				ofVec3f position(
					ofRandom(-HALF_DIM, HALF_DIM),
					ofRandom(-HALF_DIM, HALF_DIM),
					ofRandom(-HALF_DIM, HALF_DIM)
				);

				float speed = glm::gaussRand(60.f, 20.f);
				glm::vec3 velocity = glm::sphericalRand(speed);

				particleSystem.addParticle((nm::Particle::Type)(i % 6), position, velocity);
			}
#endif

			// TODO: EZ Look at this stuff
			 this->debug = false;
			
			 // Load shaders.
			 this->shader.load(this->getDataPath("shaders/particle"));
			 this->shader.printActiveUniforms();
			 this->shader.printActiveUniformBlocks();
			 CheckGLError();
			
			 this->skyboxShader.load(this->getDataPath("shaders/skybox"));
			 glGenVertexArrays(1, &this->defaultVao);
			 CheckGLError();
			
			 // Set up view UBO.
			 const int viewUboBinding = 1;
			 this->viewUbo.setup(viewUboBinding);
			 this->viewUbo.configureShader(this->shader);
			 this->viewUbo.configureShader(this->skyboxShader);
			 CheckGLError();
			
			 // Set up lighting.
			 this->lightingSystem.setup(this->getCamera());
			 this->lightingSystem.configureShader(this->shader);
			 this->lightingSystem.setAmbientIntensity(0.5f);
			 CheckGLError();
			
			 // Set up PBR.
			 float aperture = 0.5f;
			 float shutterSpeed = 1.0f / 60.0f;
			
			 this->exposure = ofxRTK::util::CalcEVFromCameraSettings(aperture, shutterSpeed);
			 this->gamma = 2.2f;
			 CheckGLError();
			
			 this->skyboxMap.loadDDSTexture(this->getDataPath("textures/output_skybox.dds"));
			 this->irradianceMap.loadDDSTexture(this->getDataPath("textures/output_iem.dds"));
			 this->radianceMap.loadDDSTexture(this->getDataPath("textures/output_pmrem.dds"));
			 CheckGLError();
			
			 glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		}

		//--------------------------------------------------------------
		// Clean up your crap here!
		void Particles::exit()
		{

		}

		//--------------------------------------------------------------
		// Resize your content here.
		// Note that this is not the window size but the canvas size.
		void Particles::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void Particles::update(double & dt)
		{
			photons.update();
			particleSystem.update();

			auto & pointLights = this->lightingSystem.getPointLights();
			auto & photons = this->photons.getPosnsRef();
			// Remove extra point lights.
			if (pointLights.size() > photons.size())
			{
				pointLights.resize(photons.size());
				//cout << "removing point light " << this->lightingSystem.getPointLights().size() << endl;
			}
			// Update current point lights.
			for (int i = 0; i < pointLights.size(); ++i)
			{
				pointLights[i].position = glm::vec4(photons[i], 1.0f);
			}
			// Add new point lights.
			static const auto radius = 120.0f;
			for (int i = pointLights.size(); i < photons.size(); ++i)
			{
				auto light = ofxRTK::lighting::PointLight(photons[i], glm::vec3(1.0f, 1.0f, 1.0f), radius, 60000.0f);
				//light.color = glm::normalize(glm::vec3(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f)));
				//cout << "color is " << light.color << endl;
				this->lightingSystem.addPointLight(light);
				//cout << "adding point light " << this->lightingSystem.getPointLights().size() << endl;
			}
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Particles::drawBack()
		{

		}

		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Particles::drawWorld()
		{
			ofDisableAlphaBlending();

			auto cullFaceEnabled = glIsEnabled(GL_CULL_FACE);
			GLint cullFaceMode[1];
			glGetIntegerv(GL_CULL_FACE_MODE, cullFaceMode);

			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			this->viewUbo.bind();
			{
				this->skyboxMap.bind(14);
				this->irradianceMap.bind(2);
				this->radianceMap.bind(3);

				this->viewUbo.update(this->getCamera());
				this->lightingSystem.update(this->getCamera());

				ofSetColor(255, 255, 255, 255);

				if (this->debug)
				{
					this->lightingSystem.debugDrawFrustum(this->getCamera());

					this->lightingSystem.debugDrawCulledPointLights();
					this->lightingSystem.debugDrawClusteredPointLights();
					this->lightingSystem.debugDrawOccupiedClusters(this->getCamera());

					for (auto & light : this->lightingSystem.getPointLights())
					{
						ofSetColor(light.color.r * 255, light.color.g * 255, light.color.b * 255);
						ofDrawSphere(light.position.xyz, light.radius);
					}
				}
				else
				{
					//this->drawSkybox();

					this->lightingSystem.begin();
					{
						this->shader.begin();
						{
							this->material.setUniforms(this->shader);
							this->shader.setUniform1f("uExposure", this->exposure);
							this->shader.setUniform1f("uGamma", this->gamma);
							this->shader.setUniform1i("uIrradianceMap", 2);
							this->shader.setUniform1i("uRadianceMap", 3);

							//this->drawScene();
							particleSystem.draw(this->shader);
						}
						this->shader.end();
					}
					this->lightingSystem.end();
				}

				this->skyboxMap.unbind(14);
				this->irradianceMap.unbind(2);
				this->radianceMap.unbind(3);
			}
			this->viewUbo.unbind();

			//particleSystem.drawWalls();
			glDepthMask(GL_FALSE);
			photons.draw();
			glDepthMask(GL_TRUE);

			// Restore state.
			if (GL_TRUE == cullFaceEnabled)
			{
				glCullFace(cullFaceMode[0]);
			}
			else
			{
				glDisable(GL_CULL_FACE);
			}
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Particles::drawFront()
		{

		}

		//--------------------------------------------------------------
		void Particles::drawSkybox()
		{
			glDisable(GL_CULL_FACE);
			ofDisableDepthTest();

			this->skyboxShader.begin();
			this->skyboxShader.setUniform1f("uExposure", this->exposure);
			this->skyboxShader.setUniform1f("uGamma", this->gamma);
			this->skyboxShader.setUniform1i("uCubeMap", 3);
			{
				// Draw full-screen quad.
				glBindVertexArray(this->defaultVao);
				glDrawArrays(GL_TRIANGLES, 0, 3);
			}
			this->skyboxShader.end();

			ofEnableDepthTest();
			glEnable(GL_CULL_FACE);
		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void Particles::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("State", settings))
			{
				if (ImGui::Button("Save"))
				{
					static const string & filename = "state.json";
					if (this->saveState(this->getCurrentPresetPath(filename)))
					{
						this->parameters.stateFile = filename;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Load State", false, this->getPresetPath());
					if (result.bSuccess)
					{
						if (this->loadState(result.filePath))
						{
							this->parameters.stateFile = result.fileName;
						}
					}
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Rendering", settings))
			{
				ofxPreset::Gui::AddGroup(this->material.parameters, settings);
				
				if (ImGui::CollapsingHeader("Camera", nullptr, true, true))
				{
					ImGui::SliderFloat("Exposure", &this->exposure, 0.01f, 10.0f);
					ImGui::SliderFloat("Gamma", &this->gamma, 0.01f, 10.0f);
				}

				if (ImGui::CollapsingHeader(this->lightingSystem.parameters.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->lightingSystem.ambientIntensity);
					ImGui::Checkbox("Debug", &this->debug);

					ImGui::BeginGroup();
					ImGui::Text("Stats");
					ImGui::Text("Visible Lights: %u", this->lightingSystem.getNumVisibleLights());
					ImGui::Text("Culled Lights: %u", this->lightingSystem.getNumCulledPointLights());
					ImGui::Text("Num Affected Clusters: %u", this->lightingSystem.getNumAffectedClusters());
					ImGui::Text("Num Light Indices: %u", this->lightingSystem.getNumPointLightIndices());
					ImGui::EndGroup();
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			ofxPreset::Gui::AddGroup(this->environment->parameters, settings);
		}

		//--------------------------------------------------------------
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void Particles::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->environment->parameters);
			ofxPreset::Serializer::Serialize(json, this->lightingSystem.parameters);
			ofxPreset::Serializer::Serialize(json, this->material.parameters);
		}

		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void Particles::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->environment->parameters);
			ofxPreset::Serializer::Deserialize(json, this->lightingSystem.parameters);
			ofxPreset::Serializer::Deserialize(json, this->material.parameters);

			if (!this->parameters.stateFile->empty())
			{
				this->loadState(this->parameters.stateFile);
			}
		}

		//--------------------------------------------------------------
		bool Particles::saveState(const string & path)
		{
			auto stateFile = ofFile(path, ofFile::WriteOnly);
			nlohmann::json json;
			{
				this->particleSystem.serialize(json);
			}
			stateFile << json;

			return true;
		}

		//--------------------------------------------------------------
		bool Particles::loadState(const string & path)
		{
			auto stateFile = ofFile(path);
			if (stateFile.exists())
			{
				nlohmann::json json;
				stateFile >> json;

				this->particleSystem.deserialize(json);

				return true;
			}
			return false;
		}
	}
}
