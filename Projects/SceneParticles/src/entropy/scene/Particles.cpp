#include "Particles.h"
#include "glm/gtc/random.hpp"

#include "entropy/Helpers.h"
#include <regex>

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
		{}

		//--------------------------------------------------------------
		Particles::~Particles()
		{}

        void Particles::compileShader(){
            ofFile vertFile(this->getDataPath("shaders/particle.vert"));
            ofBuffer vertSource(vertFile);

            std::regex re_color_per_type("#define COLOR_PER_TYPE [0-1]");
            vertSource.set(std::regex_replace(vertSource.getText(), re_color_per_type, "#define COLOR_PER_TYPE " + ofToString(colorsPerType)));

            // Load shaders.
            ofShader::TransformFeedbackSettings settings;
            settings.bindDefaults = false;
            settings.shaderSources[GL_VERTEX_SHADER] = vertSource.getText();
            settings.varyingsToCapture = {"out_position", "out_color"};
            settings.sourceDirectoryPath = this->getDataPath("shaders");
            this->shader.setup(settings);
            this->shader.printActiveUniforms();
            this->shader.printActiveUniformBlocks();
        }

		//--------------------------------------------------------------
		void Particles::setup()
		{
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

            renderer.fogMaxDistance.setMax(HALF_DIM * 10);
            renderer.fogMinDistance.setMax(HALF_DIM);


			// TODO: EZ Look at this stuff
			 this->debug = false;

            compileShader();

            colorsPerTypeListener = colorsPerType.newListener([&](bool &){
                compileShader();
            });

             renderer.setup();
             feedbackBuffer.allocate(1024*1024*100, GL_STATIC_DRAW);
             feedbackVbo.setVertexBuffer(feedbackBuffer, 4, sizeof(glm::vec4) * 2, 0);
             feedbackVbo.setColorBuffer(feedbackBuffer, sizeof(glm::vec4) * 2, sizeof(glm::vec4));
             glGenQueries(1, &numPrimitivesQuery);
		}

		//--------------------------------------------------------------
		void Particles::update(double dt)
		{
            if(ofGetFrameNum()%2==0){
                photons.update();
                particleSystem.update();

                auto & photons = this->photons.getPosnsRef();
                // Remove extra point lights.
                if (pointLights.size() > photons.size())
                {
                    pointLights.resize(photons.size());
                    cout << "removing point light " << this->pointLights.size() << endl;
                }
                // Update current point lights.
                for (int i = 0; i < pointLights.size(); ++i)
                {
                    pointLights[i].setPosition(photons[i]);
                    //cout << "updating point light " << i << " " << pointLights[i].position << endl;
                }
                // Add new point lights.
                static const auto radius = 120.0f;
                for (auto &light: pointLights)
                {
                    light.enable();
                }


                ofShader::TransformFeedbackBinding binding(feedbackBuffer);
                binding.index = 0;
                binding.offset = 0;
                binding.size = feedbackBuffer.size();
                glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, numPrimitivesQuery);
                this->shader.beginTransformFeedback(GL_TRIANGLES, binding);
                {
                    particleSystem.draw(this->shader);
                }
                this->shader.endTransformFeedback(binding);
                glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
                glGetQueryObjectuiv(numPrimitivesQuery, GL_QUERY_RESULT, &numPrimitives);
            }
		}

		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Particles::drawBackWorld()
		{
            if(additiveBlending){
                ofEnableBlendMode(OF_BLENDMODE_ADD);
            }

            renderer.draw(feedbackVbo, 0, numPrimitives * 3);
		}

		//--------------------------------------------------------------
		void Particles::drawSkybox()
		{
            /*glDisable(GL_CULL_FACE);
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
            glEnable(GL_CULL_FACE);*/
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
                ofxPreset::Gui::AddGroup(renderer.parameters, settings);
                ofxPreset::Gui::AddParameter(this->colorsPerType);
                ofxPreset::Gui::AddParameter(this->additiveBlending);
            }
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			ofxPreset::Gui::AddGroup(this->environment->parameters, settings);
		}

		//--------------------------------------------------------------
		void Particles::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->environment->parameters);
            ofxPreset::Serializer::Serialize(json, this->renderer.parameters);
		}

		//--------------------------------------------------------------
		void Particles::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->environment->parameters);
            ofxPreset::Serializer::Deserialize(json, this->renderer.parameters);

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
