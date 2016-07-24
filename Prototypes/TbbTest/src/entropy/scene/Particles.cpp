#include "Particles.h"
#include "glm/gtc/random.hpp"

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

			universe = nm::Universe::Ptr(new nm::Universe());
			particleSystem.init(universe, glm::vec3(-HALF_DIM), glm::vec3(HALF_DIM));
			photons.init(universe, glm::vec3(-HALF_DIM), glm::vec3(HALF_DIM));

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

			for (unsigned i = 0; i < nm::ParticleSystem::NUM_LIGHTS; ++i)
			{
				particleSystem.lights[i].intensity = 1.f;
				particleSystem.lights[i].radius = 1.f;
				particleSystem.lights[i].color.set(1.f, 1.f, 1.f, 1.f);

				string iStr = ofToString(i);
				persistent.add("lightPosns" + iStr, particleSystem.lights[i].position, glm::vec3(-2000.f), glm::vec3(2000.f));
				persistent.add("lightIntensities" + iStr, particleSystem.lights[i].intensity, 0.f, 5.f);
				persistent.add("lightRadiuses" + iStr, particleSystem.lights[i].radius, 0.f, 4000.f);
				persistent.add("lightCols" + iStr, particleSystem.lights[i].color, ofFloatColor(0.f), ofFloatColor(1.f));
			}
			persistent.add("roughness", particleSystem.roughness, 0.f, 1.f);
			persistent.add("universe->age", universe->getAgeRef(), 0.f, 1.f);
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
			particleSystem.draw();
			particleSystem.drawWalls();
			glDepthMask(GL_FALSE);
			photons.draw();
			glDepthMask(GL_TRUE);
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Particles::drawFront()
		{

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
