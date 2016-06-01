#include "Template.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Template::Template()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Template::~Template()
		{

		}

		//--------------------------------------------------------------
		void Template::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;
		}
		
		//--------------------------------------------------------------
		void Template::exit()
		{

		}

		//--------------------------------------------------------------
		void Template::update()
		{
			if (this->sphere.getRadius() != this->parameters.sphere.radius || this->sphere.getResolution() != this->parameters.sphere.resolution)
			{
				this->sphere.set(this->parameters.sphere.radius, this->parameters.sphere.resolution);
			}
		}
		
		//--------------------------------------------------------------
		void Template::drawWorld()
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
		void Template::gui(ofxPreset::GuiSettings & settings)
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
		void Template::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		void Template::deserialize(const nlohmann::json & json)
		{

		}
	}
}