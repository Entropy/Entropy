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
			 this->shader.load(this->getDataPath("shaders/main"));
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
			 this->material.setBaseColor(ofFloatColor(1.0f, 1.0f, 1.0f, 1.0f));
			 this->material.setMetallic(0.67f);
			 this->material.setRoughness(0.13f);
			 this->material.setEmissiveColor(ofFloatColor(1.0f, 0.4f, 0.0f, 1.0f));
			 this->material.setEmissiveIntensity(0.0f);
			 CheckGLError();
			
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

			//for (unsigned i = 0; i < nm::ParticleSystem::NUM_LIGHTS; ++i)
			//{
			//	particleSystem.lights[i].intensity = 1.f;
			//	particleSystem.lights[i].radius = 1.f;
			//	particleSystem.lights[i].color.set(1.f, 1.f, 1.f, 1.f);

			//	string iStr = ofToString(i);
			//	persistent.add("lightPosns" + iStr, particleSystem.lights[i].position, glm::vec3(-2000.f), glm::vec3(2000.f));
			//	persistent.add("lightIntensities" + iStr, particleSystem.lights[i].intensity, 0.f, 5.f);
			//	persistent.add("lightRadiuses" + iStr, particleSystem.lights[i].radius, 0.f, 4000.f);
			//	persistent.add("lightCols" + iStr, particleSystem.lights[i].color, ofFloatColor(0.f), ofFloatColor(1.f));
			//}
			//persistent.add("roughness", particleSystem.roughness, 0.f, 1.f);
			persistent.add("environment.energy", environment->getEnergyRef(), 0.f, 1.f);
			persistent.add("environment.forceMultiplierMin", environment->getForceMultiplierMinRef(), 1e7, 1e8);
			persistent.add("environment.forceMultiplierMax", environment->getForceMultiplierMaxRef(), 1e7, 1e8);
			persistent.add("environment.annihilationThreshMin", environment->getAnnihilationThreshMinRef(), 0.f, 1.f);
			persistent.add("environment.annihilationThreshMax", environment->getAnnihilationThreshMaxRef(), 0.f, 1.f);
			persistent.add("environment.fusionThreshExponentMin", environment->getFusionThreshExponentMinRef(), -6.f, -5.f);
			persistent.add("environment.fusionThreshExponentMax", environment->getFusionThreshExponentMaxRef(), -6.f, -5.f);
			persistent.add("environment.pairProductionThreshMin", environment->getPairProductionThreshMinRef(), 0.f, 1.f);
			persistent.add("environment.pairProductionThreshMax", environment->getPairProductionThreshMaxRef(), 0.f, 1.f);

			persistent.load("settings/settings.xml");
		}

		//--------------------------------------------------------------
		// Clean up your crap here!
		void Particles::exit()
		{
			persistent.save("settings/settings.xml");
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
				cout << "removing point light " << this->lightingSystem.getPointLights().size() << endl;
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
				cout << "color is " << light.color << endl;
				this->lightingSystem.addPointLight(light);
				cout << "adding point light " << this->lightingSystem.getPointLights().size() << endl;
			}

			//this->animateLights();
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Particles::drawBack()
		{
			ofBackgroundGradient(ofColor::darkBlue, ofColor::skyBlue);
		}

		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Particles::drawWorld()
		{
			ofClear(0.0f, 1.0f);

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
		void Particles::drawScene()
		{
			//glCullFace(GL_FRONT);

			//int numSpheres = 8;

			//float radius = 30.0f;
			//float spacing = radius * 2.0f + 15.0f;
			//float offset = -numSpheres * spacing * 0.5f;

			//for (int z = 0; z < numSpheres; ++z)
			//{
			//	float zPercent = z / (float)(numSpheres - 1);

			//	for (int x = 0; x < numSpheres; ++x)
			//	{
			//		float xPercent = x / (float)(numSpheres - 1);
			//		this->material.metallic = std::max(zPercent, 0.001f);
			//		this->material.roughness = std::max(xPercent * xPercent, 0.001f);
			//		this->material.setUniforms(this->shader);

			//		ofPushMatrix();
			//		{
			//			ofTranslate(offset + x * spacing, radius * 2.0, offset + z * spacing);
			//			ofScale(radius);
			//			this->shader.setUniformMatrix4f("uNormalMatrix", ofGetCurrentNormalMatrix());

			//			//ofDrawSphere(glm::vec3(offset + x * spacing, /*radius * 2.*/0, offset + z * spacing), radius);
			//			//ofDrawSphere(1.0f);
			//			static ofSpherePrimitive sphere(1.0f, 24);
			//			sphere.draw();
			//		}
			//		ofPopMatrix();
			//	}
			//}

			//glCullFace(GL_BACK);
		}

		//--------------------------------------------------------------
		void Particles::createRandomLights()
		{
			//this->lightingSystem.clearPointLights();

			//const auto positionDist = 330.0f;
			//const auto radius = 60.0f;

			//const auto numPointLights = 60;
			//for (int i = 0; i < numPointLights; ++i)
			//{
			//	auto offset = ofVec3f(ofRandom(-positionDist, positionDist), 0.0f, ofRandom(-positionDist, positionDist));
			//	auto light = ofxRTK::lighting::PointLight(offset, ofVec3f(1.0f, 1.0f, 1.0f), radius, 6000.0f);
			//	light.color = ofVec3f(ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f), ofRandom(0.0f, 1.0f)).getNormalized();
			//	this->lightingSystem.addPointLight(light);
			//}
		}

		//--------------------------------------------------------------
		void Particles::animateLights()
		{
			//auto & pointLights = this->lightingSystem.getPointLights();
			//for (int idx = 0; idx < pointLights.size(); ++idx)
			//{
			//	auto & light = pointLights[idx];
			//	light.position.y = (sinf((ofGetElapsedTimeMillis() + idx * 40) / 1400.0f) * 0.5f + 0.5f) * 100.0f;
			//}
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
				if (ImGui::CollapsingHeader("Material", nullptr, true, true))
				{
					ImGui::ColorEdit4("Base Color", (float *)&this->material.baseColor);
					ImGui::SliderFloat("Emissive Intensity", &this->material.emissiveIntensity, 0.0f, 1.0f);
					ImGui::ColorEdit4("Emissive Color", (float *)&this->material.emissiveColor);
					ImGui::SliderFloat("Metallic", &this->material.metallic, 0.0f, 1.0f);
					ImGui::SliderFloat("Roughness", &this->material.roughness, 0.0f, 1.0f);
				}

				if (ImGui::CollapsingHeader("Camera", nullptr, true, true))
				{
					ImGui::SliderFloat("Exposure", &this->exposure, 0.01f, 10.0f);
					ImGui::SliderFloat("Gamma", &this->gamma, 0.01f, 10.0f);
				}

				if (ImGui::CollapsingHeader("Lighting", nullptr, true, true))
				{
					ImGui::SliderFloat("Ambient IBL Strength", &this->lightingSystem.ambientIntensity, 0.0f, 3.0f);
					if (ImGui::Button("Create Point Lights"))
					{
						createRandomLights();
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear Point Lights"))
					{
						this->lightingSystem.clearPointLights();
					}
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
			if (ofxPreset::Gui::BeginWindow("Persistent", settings))
			{
				for (auto& pair : persistent.getFloats())
				{
					ImGui::SliderFloat(pair.first.c_str(), pair.second.getValue(), pair.second.getMin(), pair.second.getMax());
				}
				for (auto& pair : persistent.getVec2fs())
				{
					ImGui::SliderFloat2(pair.first.c_str(), &pair.second.getValue()->x, pair.second.getMin().x, pair.second.getMax().x);
				}
				for (auto& pair : persistent.getVec3fs())
				{
					ImGui::SliderFloat3(pair.first.c_str(), &pair.second.getValue()->x, pair.second.getMin().x, pair.second.getMax().x);
				}
				for (auto& pair : persistent.getBools())
				{
					ImGui::Checkbox(pair.first.c_str(), pair.second.getValue());
				}
				for (auto& pair : persistent.getFloatColors())
				{
					ImGui::ColorEdit4(pair.first.c_str(), &pair.second.getValue()->r);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void Particles::serialize(nlohmann::json & json)
		{

		}

		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void Particles::deserialize(const nlohmann::json & json)
		{
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
