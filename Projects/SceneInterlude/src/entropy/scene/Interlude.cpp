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
		void Interlude::drawBackWorld()
		{
			for (auto instance : this->stripes[render::Layout::Back])
			{
				if (instance->enabled)
				{
					instance->alpha.set(this->parameters.stripes.backAlpha);
					instance->draw();
				}
			}
		}

		//--------------------------------------------------------------
		void Interlude::drawFrontWorld()
		{
			for (auto instance : this->stripes[render::Layout::Front])
			{
				if (instance->enabled)
				{
					instance->alpha.set(this->parameters.stripes.frontAlpha);
					instance->draw();
				}
			}
		}

		//--------------------------------------------------------------
		void Interlude::gui(ofxImGui::Settings & settings)
		{
			ofxImGui::SetNextWindow(settings);
			if (ofxImGui::BeginWindow(this->parameters.getName().c_str(), settings))
			{
				if (ofxImGui::BeginTree(this->parameters.stripes, settings))
				{
					ofxImGui::AddParameter(this->parameters.stripes.backAlpha);
					ofxImGui::AddParameter(this->parameters.stripes.frontAlpha);

					for (auto & it : this->stripes)
					{
						const string name = (it.first == render::Layout::Back ? "Back Stripes" : "Front Stripes");
						if (ofxImGui::BeginTree(name, settings))
						{
							ImGui::ListBoxHeader("List", 3);
							for (auto i = 0; i < it.second.size(); ++i)
							{
								auto name = "Stripes " + ofToString(i);
								ImGui::Checkbox(name.c_str(), &this->openGuis[it.first][i]);
							}
							ImGui::ListBoxFooter();

							if (it.second.size() < MAX_NUM_STRIPES)
							{
								if (ImGui::Button("Add Stripes"))
								{
									this->addStripes(it.first);
								}
							}
							if (!it.second.empty())
							{
								ImGui::SameLine();
								if (ImGui::Button("Remove Stripes"))
								{
									this->removeStripes(it.first);
								}
							}

							ofxImGui::EndTree(settings);
						}
					}

					ofxImGui::EndTree(settings);
				}
			}
			ofxImGui::EndWindow(settings);

			// Move to the next column for the Stripes gui windows.
			auto stripesSettings = ofxImGui::Settings();
			stripesSettings.windowPos = glm::vec2(800.0f + kGuiMargin, 0.0f);
			for (auto & it : this->stripes)
			{
				const string prefix = (it.first == render::Layout::Back ? "Back " : "Front ");
				for (auto i = 0; i < it.second.size(); ++i)
				{
					if (this->openGuis[it.first][i])
					{
						auto instance = it.second.at(i);
						const auto name = prefix + instance->parameters.getName();

						ofxImGui::SetNextWindow(stripesSettings);
						if (ofxImGui::BeginWindow(name, stripesSettings, false, &this->openGuis[it.first][i]))
						{
							ofxImGui::AddParameter(instance->enabled);
							static const vector<string> blendLabels{ "Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen" };
							ofxImGui::AddRadio(instance->blendMode, blendLabels, 3);
							ofxImGui::AddParameter(instance->depthTest);
							static const vector<string> cullLabels{ "None", "Back", "Front" };
							ofxImGui::AddRadio(instance->cullFace, cullLabels, 3);
							ofxImGui::AddParameter(instance->color, false);
							//ofxImGui::AddParameter(instance->alpha);
							ofxImGui::AddParameter(instance->lineWidth);
							ofxImGui::AddParameter(instance->lineHeight);
							ofxImGui::AddParameter(instance->lineCount);
							ofxImGui::AddParameter(instance->spaceWidth);
							ofxImGui::AddParameter(instance->zPosition);
						}
						ofxImGui::EndWindow(stripesSettings);
					}
				}
			}

			settings.totalBounds.growToInclude(stripesSettings.totalBounds);
			settings.mouseOverGui |= stripesSettings.mouseOverGui;
		}

		//--------------------------------------------------------------
		void Interlude::serialize(nlohmann::json & json)
		{
			// Save Stripes.
			for (auto & it : this->stripes)
			{
				for (auto & it : this->stripes)
				{
					const string name = (it.first == render::Layout::Back ? "Stripes Back" : "Stripes Front");
					auto & jsonStripes = json[name];

					for (auto instance : it.second)
					{
						ofxPreset::Serializer::Serialize(jsonStripes, instance->parameters);
					}
				}
			}
		}
		
		//--------------------------------------------------------------
		void Interlude::deserialize(const nlohmann::json & json)
		{
			// Restore Stripes.
			if (json.count("Stripes Back"))
			{
				auto layout = render::Layout::Back;
				auto jsonStripes = json["Stripes Back"];
				for (int i = 0; i < jsonStripes.size(); ++i)
				{
					auto instance = this->addStripes(layout);
					if (instance)
					{
						ofxPreset::Serializer::Deserialize(jsonStripes, instance->parameters);
					}
				}
			}
			if (json.count("Stripes Front"))
			{
				auto layout = render::Layout::Front;
				auto jsonStripes = json["Stripes Front"];
				for (int i = 0; i < jsonStripes.size(); ++i)
				{
					auto instance = this->addStripes(layout);
					if (instance)
					{
						ofxPreset::Serializer::Deserialize(jsonStripes, instance->parameters);
					}
				}
			}
		}

		//--------------------------------------------------------------
		std::shared_ptr<geom::Stripes> Interlude::addStripes(render::Layout layout)
		{
			auto instance = std::make_shared<geom::Stripes>();
			this->stripes[layout].push_back(instance);

			auto idx = this->stripes[layout].size() - 1;
			auto name = "Stripes " + ofToString(idx);
			instance->parameters.setName(name);

			this->openGuis[layout][idx] = false;

			return instance;
		}

		//--------------------------------------------------------------
		void Interlude::removeStripes(render::Layout layout)
		{
			this->stripes[layout].pop_back();
		}
	}
}