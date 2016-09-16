#include "Interlude.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Interlude::Interlude()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Interlude::~Interlude()
		{
			this->clear();
		}

		//--------------------------------------------------------------
		void Interlude::init()
		{
			// Init parameters.
			this->parameters.setName("Interlude");
		}

		//--------------------------------------------------------------
		void Interlude::clear()
		{

		}

		//--------------------------------------------------------------
		void Interlude::setup()
		{

		}
		
		//--------------------------------------------------------------
		void Interlude::exit()
		{

		}

		//--------------------------------------------------------------
		void Interlude::resizeBack(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void Interlude::resizeFront(ofResizeEventArgs & args)
		{

		}

		//--------------------------------------------------------------
		void Interlude::update(double dt)
		{
			
		}

		//--------------------------------------------------------------
		void Interlude::drawFrontWorld()
		{
			for (auto instance : this->stripes)
			{
				if (instance->enabled)
				{
					instance->draw();
				}
			}
		}

		//--------------------------------------------------------------
		void Interlude::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings))
			{
				if (ofxPreset::Gui::BeginTree("Stripes", settings))
				{
					ImGui::ListBoxHeader("List", 3);
					for (auto i = 0; i < this->stripes.size(); ++i)
					{
						auto name = "Stripes " + ofToString(i);
						ImGui::Checkbox(name.c_str(), &this->openGuis[i]);
					}
					ImGui::ListBoxFooter();

					if (this->stripes.size() < MAX_NUM_STRIPES)
					{
						if (ImGui::Button("Add Stripes"))
						{
							this->addStripes();
						}
					}
					if (!this->stripes.empty())
					{
						ImGui::SameLine();
						if (ImGui::Button("Remove Stripes"))
						{
							this->removeStripes();
						}
					}

					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			// Move to the next column for the Stripes gui windows.
			auto stripesSettings = ofxPreset::Gui::Settings();
			stripesSettings.windowPos = glm::vec2(800.0f + kGuiMargin, 0.0f);
			for (auto i = 0; i < this->stripes.size(); ++i)
			{
				if (this->openGuis[i])
				{
					auto instance = this->stripes[i];

					ofxPreset::Gui::SetNextWindow(stripesSettings);
					if (ofxPreset::Gui::BeginWindow(instance->parameters.getName(), stripesSettings, false, &this->openGuis[i]))
					{
						ofxPreset::Gui::AddParameter(instance->enabled);
						static const vector<string> blendLabels{ "Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen" };
						ofxPreset::Gui::AddRadio(instance->blendMode, blendLabels, 3);
						ofxPreset::Gui::AddParameter(instance->depthTest);
						static const vector<string> cullLabels{ "None", "Back", "Front" };
						ofxPreset::Gui::AddRadio(instance->cullFace, cullLabels, 3);
						ofxPreset::Gui::AddParameter(instance->color);
						ofxPreset::Gui::AddParameter(instance->alpha);
						ofxPreset::Gui::AddParameter(instance->lineWidth);
						ofxPreset::Gui::AddParameter(instance->lineHeight);
						ofxPreset::Gui::AddParameter(instance->spaceWidth);
						ofxPreset::Gui::AddParameter(instance->count);
						ofxPreset::Gui::AddParameter(instance->zPosition);
					}
					ofxPreset::Gui::EndWindow(stripesSettings);
				}
			}

			settings.totalBounds.growToInclude(stripesSettings.totalBounds);
			settings.mouseOverGui |= stripesSettings.mouseOverGui;
		}

		//--------------------------------------------------------------
		void Interlude::serialize(nlohmann::json & json)
		{
			// Save Stripes.
			auto & jsonStripes = json["Stripes"];
			for (auto instance : this->stripes)
			{
				ofxPreset::Serializer::Serialize(jsonStripes, instance->parameters);
			}
		}
		
		//--------------------------------------------------------------
		void Interlude::deserialize(const nlohmann::json & json)
		{
			// Restore Stripes.
			if (json.count("Stripes"))
			{
				auto jsonStripes = json["Stripes"];
				for (int i = 0; i < jsonStripes.size(); ++i)
				{
					auto instance = this->addStripes();
					if (instance)
					{
						ofxPreset::Serializer::Deserialize(jsonStripes, instance->parameters);
					}
				}
			}
		}

		//--------------------------------------------------------------
		std::shared_ptr<geom::Stripes> Interlude::addStripes()
		{
			auto instance = std::make_shared<geom::Stripes>();
			this->stripes.push_back(instance);

			auto idx = this->stripes.size() - 1;
			auto name = "Stripes " + ofToString(idx);
			instance->parameters.setName(name);

			this->openGuis[idx] = false;

			return instance;
		}

		//--------------------------------------------------------------
		void Interlude::removeStripes()
		{
			this->stripes.pop_back();
		}
	}
}