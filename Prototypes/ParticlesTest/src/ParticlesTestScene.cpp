#include "ParticlesTestScene.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		ParticlesTestScene::ParticlesTestScene() : 
			Base(),
			roughness(0.05f)
		{
			ENTROPY_SCENE_SETUP_LISTENER;
			for (unsigned i = 0; i < NUM_LIGHTS; ++i)
			{
				lightIntensities[i] = 1.f;
				lightRadiuses[i] = 1.f;
				lightCols[i].set(1.f, 1.f, 1.f, 1.f);

				string iStr = ofToString(i);
				persistent.add("lightPosns" + iStr, lightPosns[i], ofVec3f(-2000.f), ofVec3f(2000.f));
				persistent.add("lightIntensities" + iStr, lightIntensities[i], 0.f, 5.f);
				persistent.add("lightRadiuses" + iStr, lightRadiuses[i], 0.f, 4000.f);
				persistent.add("lightCols" + iStr, lightCols[i], ofFloatColor(0.f), ofFloatColor(1.f));
			}
			persistent.add("roughness", roughness, 0.f, 1.f);
		}
		
		//--------------------------------------------------------------
		ParticlesTestScene::~ParticlesTestScene()
		{

		}

		//--------------------------------------------------------------
		// Set up your crap here!
		void ParticlesTestScene::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			particleSystem.init(1600, 1600, 1600);

			for (int i = 0; i < ParticleSystem::MAX_PARTICLES; ++i)
			{
				float mass = ofRandom(0.01f, 0.1f);
				float radius = ofMap(mass, 0.01f, 0.1f, 1.0f, 6.0f);

				particleSystem.addParticle(
					ofVec3f(ofRandom(-500.0f, 500.0f), ofRandom(-500.0f, 500.0f), ofRandom(-500.0f, 500.0f)),
					ofVec3f(ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f), ofRandom(-1.0f, 1.0f)),
					mass, 
					radius
				);
			}

			particleShader.load("shaders/particle");
		}
		
		//--------------------------------------------------------------
		// Clean up your crap here!
		void ParticlesTestScene::exit()
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void ParticlesTestScene::update(double & dt)
		{
			if (ofGetFrameNum() % 2 == 0) particleSystem.step((1.0f / 60.0f * 1000.0f) * 2.0f);
			particleSystem.update();
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void ParticlesTestScene::drawBack()
		{

		}
		
		//--------------------------------------------------------------
		// Draw 3D elements here.
		void ParticlesTestScene::drawWorld()
		{
			cam.begin();
			{
				// draw bounds
				particleSystem.debugDrawWorldBounds();

				// draw particles
				particleShader.begin();
				{
					particleShader.setUniformTexture("uOffsetTex", particleSystem.getPositionTexture(), 0);
					particleShader.setUniform1i("numLights", NUM_LIGHTS);
					particleShader.setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
					particleShader.setUniform1f("roughness", roughness);
					for (int i = 0; i < NUM_LIGHTS; i++)
					{
						string index = ofToString(i);
						particleShader.setUniform3f("lights[" + index + "].position", lightPosns[i] * ofGetCurrentViewMatrix());
						particleShader.setUniform4f("lights[" + index + "].color", lightCols[i]);
						particleShader.setUniform1f("lights[" + index + "].intensity", lightIntensities[i]);
						particleShader.setUniform1f("lights[" + index + "].radius", lightRadiuses[i]);
					}
					particleSystem.debugDrawParticles();
					particleShader.end();
				}
			}
			cam.end();
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void ParticlesTestScene::drawFront()
		{

		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void ParticlesTestScene::gui(ofxPreset::GuiSettings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
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
		void ParticlesTestScene::serialize(nlohmann::json & json)
		{
			persistent.save("settings/particles.xml");
		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void ParticlesTestScene::deserialize(const nlohmann::json & json)
		{
			persistent.load("settings/particles.xml");
		}
	}
}