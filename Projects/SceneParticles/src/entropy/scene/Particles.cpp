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

			// Register Environment and Renderer parameters for Mappings and serialization.
			this->parameters.add(this->environment->parameters);
			this->parameters.add(this->renderers[entropy::render::Layout::Back].parameters);
			this->parameters.add(this->renderers[entropy::render::Layout::Front].parameters);

			this->parameterListeners.push_back(parameters.ambientLight.newListener([&](float & ambient)
			{
				ofSetGlobalAmbientColor(ofFloatColor(ambient));
			}));

			this->renderers[entropy::render::Layout::Back].fogMaxDistance.setMax(HALF_DIM * 10);
			this->renderers[entropy::render::Layout::Back].fogMinDistance.setMax(HALF_DIM);

			this->renderers[entropy::render::Layout::Front].fogMaxDistance.setMax(HALF_DIM * 10);
			this->renderers[entropy::render::Layout::Front].fogMinDistance.setMax(HALF_DIM);

			 this->debug = false;

			// Load shaders.
			shaderSettings.bindDefaults = false;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = this->getDataPath("shaders/particle.vert");
			shaderSettings.varyingsToCapture = { "out_position", "out_color", "out_normal" };
			shaderSettings.sourceDirectoryPath = this->getDataPath("shaders");
			shaderSettings.intDefines["COLOR_PER_TYPE"] = this->parameters.colorsPerType;
			this->shader.setup(shaderSettings);

			// Add parameter listeners.
			this->parameterListeners.push_back(this->parameters.colorsPerType.newListener([&](bool & colorsPerType)
			{
				shaderSettings.intDefines["COLOR_PER_TYPE"] = colorsPerType;
				this->shader.setup(shaderSettings);
			}));

			this->renderers[entropy::render::Layout::Back].setup(400);
			this->renderers[entropy::render::Layout::Front].setup(400);

			 for(auto & light: pointLights){
				 light.setup();
				 light.setAmbientColor(ofFloatColor::black);
				 light.setSpecularColor(ofFloatColor::white);
			 }
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
			ofSetGlobalAmbientColor(ofFloatColor(parameters.ambientLight));

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
		void Particles::resizeBack(ofResizeEventArgs & args){
			this->renderers[render::Layout::Back].resize(GetCanvas(render::Layout::Back)->getWidth(), GetCanvas(render::Layout::Back)->getHeight());
		}

		//--------------------------------------------------------------
		void Particles::resizeFront(ofResizeEventArgs & args){
			this->renderers[render::Layout::Front].resize(GetCanvas(render::Layout::Front)->getWidth(), GetCanvas(render::Layout::Front)->getHeight());
		}

		//--------------------------------------------------------------
		void Particles::exit()
		{
			particleSystem.clearParticles();
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
                        light.setDiffuseColor(ofFloatColor::white * parameters.lightStrength);
                        light.setPointLight();
                        light.setPosition(photons[i]);
                        light.setAttenuation(0,0,parameters.attenuation);
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
				if (this->parameters.additiveBlending)
				{
					ofEnableBlendMode(OF_BLENDMODE_ADD);
				}

				auto & camera = getCamera(layout)->getEasyCam();
				this->renderers[layout].draw(feedbackVbo, 0, numPrimitives * 3, camera);
				if (parameters.drawPhotons)
				{
					photons.draw();
				}
			}
		}

		//--------------------------------------------------------------
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
				if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Back].parameters, settings))
				{
					ofxPreset::Gui::AddGroup(this->renderers[entropy::render::Layout::Back].parameters, settings);
					auto numPoints = 100;
					ImGui::PlotLines("Fog funtion", this->renderers[entropy::render::Layout::Back].getFogFunctionPlot(numPoints).data(), numPoints);
					this->renderers[entropy::render::Layout::Back].clip = this->renderers[entropy::render::Layout::Back].sphericalClip ||
																		  this->renderers[entropy::render::Layout::Back].wobblyClip;
				}

				if (ofxPreset::Gui::BeginTree(this->renderers[render::Layout::Front].parameters, settings))
				{
					ofxPreset::Gui::AddGroup(this->renderers[entropy::render::Layout::Front].parameters, settings);
					auto numPoints = 100;
					ImGui::PlotLines("Fog funtion", this->renderers[entropy::render::Layout::Front].getFogFunctionPlot(numPoints).data(), numPoints);
					this->renderers[entropy::render::Layout::Front].clip = this->renderers[entropy::render::Layout::Front].sphericalClip ||
																		   this->renderers[entropy::render::Layout::Front].wobblyClip;
				}

                ofxPreset::Gui::AddParameter(this->parameters.colorsPerType);
                ofxPreset::Gui::AddParameter(this->parameters.additiveBlending);
                ofxPreset::Gui::AddParameter(this->parameters.ambientLight);
                ofxPreset::Gui::AddParameter(this->parameters.attenuation);
                ofxPreset::Gui::AddParameter(this->parameters.lightStrength);
                ofxPreset::Gui::AddParameter(this->parameters.drawPhotons);
                ImGui::Checkbox("debug lights", &debug);
            }
			ofxPreset::Gui::AddGroup(nm::Particle::parameters, settings);
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			ofxPreset::Gui::AddGroup(this->environment->parameters, settings);
		}

		//--------------------------------------------------------------
		void Particles::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, nm::Particle::parameters);
		}

		//--------------------------------------------------------------
		void Particles::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, nm::Particle::parameters);

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
