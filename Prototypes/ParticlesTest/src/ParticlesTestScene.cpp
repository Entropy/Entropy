#include "ParticlesTestScene.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		ParticlesTestScene::ParticlesTestScene()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
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

			for (int i = 0; i < 20000; ++i)
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
			/*if (this->sphere.getRadius() != this->parameters.sphere.radius || this->sphere.getResolution() != this->parameters.sphere.resolution)
			{
				this->sphere.set(this->parameters.sphere.radius, this->parameters.sphere.resolution);
			}*/

			if (ofGetFrameNum() % 2 == 0)
			{
				particleSystem.step((1.0f / 60.0f * 1000.0f) * 2.0f);
			}

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
			particleSystem.debugDrawWorldBounds();
			particleSystem.debugDrawParticles();
			cam.end();

			/*
			ofPushStyle();
			{
				ofSetColor(this->parameters.sphere.color.get());
				if (this->parameters.sphere.filled)
				{
					this->sphere.draw(OF_MESH_FILL);
				}
				else
				{
					this->sphere.draw(OF_MESH_WIREFRAME);
				}
			}
			ofPopStyle();
			*/
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
				if (ImGui::CollapsingHeader(this->parameters.sphere.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.sphere.color);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.filled);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.radius);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.resolution);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		// Do something after the parameters are saved.
		// You can save other stuff to the same json object here too.
		void ParticlesTestScene::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void ParticlesTestScene::deserialize(const nlohmann::json & json)
		{

		}
	}
}