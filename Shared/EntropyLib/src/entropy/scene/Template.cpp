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
			ENTROPY_SCENE_DRAW_LISTENER;
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
			ofSetCircleResolution(this->parameters.circle.resolution);
		}
		
		//--------------------------------------------------------------
		void Template::draw()
		{
			ofSetColor(this->parameters.circle.color.get());
			ofDrawCircle(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f, this->parameters.circle.radius);
		}

		//--------------------------------------------------------------
		void Template::gui(ofxPreset::GuiSettings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::CollapsingHeader(this->parameters.circle.getName().c_str(), nullptr, true, true))
				{
					ofxPreset::Gui::AddParameter(this->parameters.circle.color);
					ofxPreset::Gui::AddParameter(this->parameters.circle.radius);
					ofxPreset::Gui::AddParameter(this->parameters.circle.resolution);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Template::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->parameters);
		}
		
		//--------------------------------------------------------------
		void Template::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->parameters);

			ofSetCircleResolution(this->parameters.circle.resolution);
		}
	}
}