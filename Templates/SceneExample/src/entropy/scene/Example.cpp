#include "Example.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Example::Example()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Example::~Example()
		{

		}

		//--------------------------------------------------------------
		// Set up your crap here!
		void Example::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;
		}
		
		//--------------------------------------------------------------
		// Clean up your crap here!
		void Example::exit()
		{

		}

		//--------------------------------------------------------------
		// Resize your content here. 
		// Note that this is not the window size but the canvas size.
		void Example::resize(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		// Update your data here, once per frame.
		void Example::update(double & dt)
		{
			if (this->sphere.getRadius() != this->parameters.sphere.radius || this->sphere.getResolution() != this->parameters.sphere.resolution)
			{
				this->sphere.set(this->parameters.sphere.radius, this->parameters.sphere.resolution);
			}
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the background here.
		void Example::drawBack()
		{
			ofClear(128, 255);
		}
		
		//--------------------------------------------------------------
		// Draw 3D elements here.
		void Example::drawWorld()
		{
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
		}

		//--------------------------------------------------------------
		// Draw 2D elements in the foreground here.
		void Example::drawFront()
		{

		}

		//--------------------------------------------------------------
		// Add Scene specific GUI windows here.
		void Example::gui(ofxPreset::Gui::Settings & settings)
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
		void Example::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		// Do something after the parameters are loaded.
		// You can load your other stuff here from that json object.
		// You can also set any refresh flags if necessary.
		void Example::deserialize(const nlohmann::json & json)
		{

		}
	}
}