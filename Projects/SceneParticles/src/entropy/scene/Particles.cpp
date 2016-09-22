#include "Particles.h"
#include "glm/gtc/random.hpp"

#include "entropy/Helpers.h"
#include <regex>

#define MAX_LIGHTS 16u

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		const float Particles::HALF_DIM = 400.f;

		//--------------------------------------------------------------
		Particles::Particles()
			: Base()
		{}

		//--------------------------------------------------------------
		Particles::~Particles()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Particles::init()
		{
			// Initialize particle system.
			environment = nm::Environment::Ptr(new nm::Environment(glm::vec3(-HALF_DIM), glm::vec3(HALF_DIM)));
			particleSystem.init(environment);
			photons.init(environment);
			pointLights.resize(MAX_LIGHTS);

			// Initialize transform feedback.
			feedbackBuffer.allocate(1024 * 1024 * 300, GL_STATIC_DRAW);
			auto stride = sizeof(glm::vec4) * 3;// + sizeof(glm::vec3);
			feedbackVbo.setVertexBuffer(feedbackBuffer, 4, stride, 0);
			feedbackVbo.setColorBuffer(feedbackBuffer, stride, sizeof(glm::vec4));
			feedbackVbo.setNormalBuffer(feedbackBuffer, stride, sizeof(glm::vec4) * 2);
			glGenQueries(1, &numPrimitivesQuery);

			// Setup renderers.
			this->renderers[render::Layout::Back].setup(HALF_DIM);
			this->renderers[render::Layout::Back].fogMaxDistance.setMax(HALF_DIM * 25);
			this->renderers[render::Layout::Back].fogMinDistance.setMax(HALF_DIM);
			this->renderers[render::Layout::Back].parameters.setName("Renderer Back");
			this->populateMappings(this->renderers[render::Layout::Back].parameters);

			this->renderers[render::Layout::Front].setup(HALF_DIM);
			this->renderers[render::Layout::Front].fogMaxDistance.setMax(HALF_DIM * 25);
			this->renderers[render::Layout::Front].fogMinDistance.setMax(HALF_DIM);
			this->renderers[render::Layout::Front].parameters.setName("Renderer Front");
			this->populateMappings(this->renderers[render::Layout::Front].parameters);

			// Register Environment parameters for Mappings and serialization.
			this->parameters.add(this->environment->parameters);
			
			// Load shaders.
			shaderSettings.bindDefaults = false;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = this->getDataPath("shaders/particle.vert");
			shaderSettings.varyingsToCapture = { "out_position", "out_color", "out_normal" };
			shaderSettings.sourceDirectoryPath = this->getDataPath("shaders");
			shaderSettings.intDefines["COLOR_PER_TYPE"] = this->parameters.rendering.colorsPerType;
			this->shader.setup(shaderSettings);

			// Setup lights.
			for (auto & light : pointLights)
			{
				light.setup();
				light.setAmbientColor(ofFloatColor::black);
				light.setSpecularColor(ofFloatColor::white);
			}

			// Add parameter listeners.
			this->parameterListeners.push_back(parameters.rendering.ambientLight.newListener([&](float & ambient)
			{
				ofSetGlobalAmbientColor(ofFloatColor(ambient));
			}));
			this->parameterListeners.push_back(this->parameters.rendering.colorsPerType.newListener([&](bool & colorsPerType)
			{
				shaderSettings.intDefines["COLOR_PER_TYPE"] = colorsPerType;
				this->shader.setup(shaderSettings);
			}));
			this->parameterListeners.push_back(this->parameters.rendering.colorsPerType.newListener([&](bool & colorsPerType)
			{
				shaderSettings.intDefines["COLOR_PER_TYPE"] = colorsPerType;
				this->shader.setup(shaderSettings);
			}));

			this->debug = false;
		}

		//--------------------------------------------------------------
		void Particles::clear()
		{
			// Clear transform feedback.
			glDeleteQueries(1, &numPrimitivesQuery);
		}

		//--------------------------------------------------------------
		void Particles::setup()
		{
			ofSetGlobalAmbientColor(ofFloatColor(parameters.rendering.ambientLight));

			for (unsigned i = 0; i < 4000; ++i)
			{
				glm::vec3 position = glm::vec3(
					ofRandom(-HALF_DIM, HALF_DIM),
					ofRandom(-HALF_DIM, HALF_DIM),
					ofRandom(-HALF_DIM, HALF_DIM)
				);

				float speed = glm::gaussRand(60.f, 20.f);
				glm::vec3 velocity = glm::sphericalRand(speed);

				particleSystem.addParticle((nm::Particle::Type)(i % 6), position, velocity);
			}
		}

		//--------------------------------------------------------------
		void Particles::exit()
		{
			particleSystem.clearParticles();
		}

		//--------------------------------------------------------------
		void Particles::resizeBack(ofResizeEventArgs & args){
			this->renderers[render::Layout::Back].resize(GetCanvas(render::Layout::Back)->getWidth(), GetCanvas(render::Layout::Back)->getHeight());
		}

		//--------------------------------------------------------------
		void Particles::resizeFront(ofResizeEventArgs & args){
			this->renderers[render::Layout::Front].resize(GetCanvas(render::Layout::Front)->getWidth(), GetCanvas(render::Layout::Front)->getHeight());
		}

		//--------------------------------------------------------------
		void Particles::reset()
		{
			this->exit();
			this->setup();
		}

		//--------------------------------------------------------------
		void Particles::update(double dt)
		{
            if(ofGetFrameNum()%2==0){
                photons.update();
                particleSystem.update();

				auto & photons = this->photons.getPosnsRef();

                for(auto & light: pointLights){
                    light.disable();
                    light.setPosition(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
                }

                for (size_t i = 0, j=0; i < photons.size() && j < MAX_LIGHTS; ++i)
                {

                    if(photons[i].x > glm::vec3(-HALF_DIM*2).x  &&
                       photons[i].y > glm::vec3(-HALF_DIM*2).y  &&
                       photons[i].z > glm::vec3(-HALF_DIM*2).z  &&
                       photons[i].x < glm::vec3(HALF_DIM*2).x &&
                       photons[i].y < glm::vec3(HALF_DIM*2).y &&
                       photons[i].z < glm::vec3(HALF_DIM*2).z){
                        auto & light = pointLights[j];
                        light.enable();
                        light.setDiffuseColor(ofFloatColor::white * parameters.rendering.lightStrength);
                        light.setPointLight();
                        light.setPosition(photons[i]);
                        light.setAttenuation(0,0,parameters.rendering.attenuation);
                        j++;
                    }
                }

                glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, numPrimitivesQuery);
                this->shader.beginTransformFeedback(GL_TRIANGLES, feedbackBuffer);
                {
					std::array<ofFloatColor, nm::Particle::NUM_TYPES> colors;
					std::transform(nm::Particle::DATA, nm::Particle::DATA + nm::Particle::NUM_TYPES, colors.begin(), [](const nm::Particle::Data & data){
						return data.color.get();
					});
					this->shader.setUniform4fv("colors", reinterpret_cast<float*>(colors.data()), nm::Particle::NUM_TYPES);
                    particleSystem.draw(this->shader);
                }
                this->shader.endTransformFeedback(feedbackBuffer);
                glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
                glGetQueryObjectuiv(numPrimitivesQuery, GL_QUERY_RESULT, &numPrimitives);
            }
		}

		//--------------------------------------------------------------
		void Particles::timelineBangFired(ofxTLBangEventArgs & args)
		{
			static const string kResetFlag = "reset";
			if (args.flag.compare(0, kResetFlag.size(), kResetFlag) == 0)
			{
				this->reset();
				this->timeline->stop();
			}
		}

		//--------------------------------------------------------------
		void Particles::drawBackWorld()
		{
			this->drawSystem(entropy::render::Layout::Back);
		}

		//--------------------------------------------------------------
		void Particles::drawFrontWorld()
		{
			this->drawSystem(entropy::render::Layout::Front);
		}

		//--------------------------------------------------------------
		void Particles::drawSystem(entropy::render::Layout layout)
		{
			if (debug)
			{
				for (auto &light : pointLights)
				{
					if (light.getPosition().x > glm::vec3(-HALF_DIM).x  &&
						light.getPosition().y > glm::vec3(-HALF_DIM).y  &&
						light.getPosition().z > glm::vec3(-HALF_DIM).z  &&
						light.getPosition().x < glm::vec3(HALF_DIM).x &&
						light.getPosition().y < glm::vec3(HALF_DIM).y &&
						light.getPosition().z < glm::vec3(HALF_DIM).z)
					{
						light.draw();
					}
				}
			}
			else
			{
				if (this->parameters.rendering.additiveBlending)
				{
					ofEnableBlendMode(OF_BLENDMODE_ADD);
				}

				auto & camera = getCamera(layout)->getEasyCam();
				this->renderers[layout].draw(feedbackVbo, 0, numPrimitives * 3, camera);
				if (parameters.rendering.drawPhotons)
				{
					photons.draw();
				}
			}
		}

		//--------------------------------------------------------------
		void Particles::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::Button("Reset"))
				{
					this->reset();
				}
				
				if (ofxPreset::Gui::BeginTree("State", settings))
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

					ofxPreset::Gui::EndTree(settings);
				}

				ofxPreset::Gui::AddGroup(this->environment->parameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.rendering.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(this->parameters.rendering.colorsPerType);
				ofxPreset::Gui::AddParameter(this->parameters.rendering.additiveBlending);
				ofxPreset::Gui::AddParameter(this->parameters.rendering.ambientLight);
				ofxPreset::Gui::AddParameter(this->parameters.rendering.attenuation);
				ofxPreset::Gui::AddParameter(this->parameters.rendering.lightStrength);
				ofxPreset::Gui::AddParameter(this->parameters.rendering.drawPhotons);

				ImGui::Checkbox("Debug Lights", &debug);

				ofxPreset::Gui::AddGroup(nm::Particle::parameters, settings);
				
				static const auto numPlotPoints = 100;
				
				if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Back].parameters, settings))
				{
					ofxPreset::Gui::AddGroup(this->renderers[entropy::render::Layout::Back].parameters, settings);
					ImGui::PlotLines("Fog Function", this->renderers[entropy::render::Layout::Back].getFogFunctionPlot(numPlotPoints).data(), numPlotPoints);
					this->renderers[entropy::render::Layout::Back].clip = this->renderers[entropy::render::Layout::Back].sphericalClip ||
																		  this->renderers[entropy::render::Layout::Back].wobblyClip;

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Front].parameters, settings))
				{
					ofxPreset::Gui::AddGroup(this->renderers[entropy::render::Layout::Front].parameters, settings);
					ImGui::PlotLines("Fog Function", this->renderers[entropy::render::Layout::Front].getFogFunctionPlot(numPlotPoints).data(), numPlotPoints);
					this->renderers[entropy::render::Layout::Front].clip = this->renderers[entropy::render::Layout::Front].sphericalClip ||
																		   this->renderers[entropy::render::Layout::Front].wobblyClip;
				
					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Particles::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, nm::Particle::parameters);

			// Save Renderer settings.
			auto & jsonRenderers = json["Renderers"];
			for (auto & it : this->renderers)
			{
				ofxPreset::Serializer::Serialize(jsonRenderers, it.second.parameters);
			}
		}

		//--------------------------------------------------------------
		void Particles::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, nm::Particle::parameters);

			// Restore Renderer settings.
			if (json.count("Renderers"))
			{
				auto & jsonRenderers = json["Renderers"];
				for (auto & it : this->renderers)
				{
					ofxPreset::Serializer::Deserialize(jsonRenderers, it.second.parameters);
				}
			}

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
