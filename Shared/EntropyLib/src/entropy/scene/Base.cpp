#include "Base.h"

namespace entropy
{
	namespace scene
	{
		static const string kPresetDefaultName = "_default";
			
		//--------------------------------------------------------------
		Base::Base()
		{

		}

		//--------------------------------------------------------------
		Base::~Base()
		{
			
		}

		//--------------------------------------------------------------
		void Base::setup()
		{
			this->onSetup.notifyListeners();

			// Load presets.
			this->currPreset.clear();
			this->populatePresets();
			this->loadPreset(kPresetDefaultName);

			// Setup timeline.
			this->timeline.setup();
			this->timeline.setLoopType(OF_LOOP_NONE);
			this->timeline.setFrameRate(30.0f);
			this->timeline.setDurationInSeconds(30);
			this->timeline.setAutosave(false);

			// Populate
			this->populateMappings(this->getParameters());
		}

		//--------------------------------------------------------------
		void Base::exit()
		{
			this->onExit.notifyListeners();

			// Save default preset.
			this->savePreset(kPresetDefaultName);
			
			// Remove all listeners.
			this->onSetup.removeListeners(nullptr);
			this->onExit.removeListeners(nullptr);
			this->onUpdate.removeListeners(nullptr);
			this->onDraw.removeListeners(nullptr);
			this->onGui.removeListeners(nullptr);
			this->onSerialize.removeListeners(nullptr);
			this->onDeserialize.removeListeners(nullptr);
		}

		//--------------------------------------------------------------
		void Base::update()
		{
			for (auto mapping : this->mappings)
			{
				mapping.second->update();
			}
			
			this->onUpdate.notifyListeners();
		}

		//--------------------------------------------------------------
		void Base::draw()
		{
			ofBackground(this->getParameters().base.background.get());
			
			this->onDraw.notifyListeners();
		}

		//--------------------------------------------------------------
		void Base::gui(ofxPreset::GuiSettings & settings)
		{
			auto & parameters = this->getParameters();

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Presets", settings))
			{
				if (ImGui::Button("Save..."))
				{
					auto name = ofSystemTextBoxDialog("Enter a name for the preset", "");
					if (!name.empty())
					{
						this->savePreset(name);
					}
				}

				ImGui::ListBoxHeader("Load", 3);
				for (auto & name : this->presets)
				{
					if (ImGui::Selectable(name.c_str(), (name == this->currPreset)))
					{
						this->loadPreset(name);
					}
				}
				ImGui::ListBoxFooter();
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Mappings", settings))
			{
				for (auto it : this->mappings)
				{
					auto mapping = it.second;
					if (ofxPreset::Gui::AddParameter(mapping->animated))
					{
						mapping->animated.update();
						if (mapping->animated)
						{
							mapping->addTrack(this->timeline);
						}
						else
						{
							mapping->removeTrack(this->timeline);
						}
					}
				}
			}
			ofxPreset::Gui::EndWindow(settings);

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(parameters.base.getName(), settings))
			{
				ofxPreset::Gui::AddParameter(parameters.base.background);
			}
			ofxPreset::Gui::EndWindow(settings);

			this->onGui.notifyListeners(settings);
		}

		//--------------------------------------------------------------
		void Base::serialize(nlohmann::json & json)
		{
			this->onSerialize.notifyListeners(json);

			auto & jsonGroup = json["Presets"];
			for (auto it : this->mappings)
			{
				ofxPreset::Serializer::Serialize(jsonGroup, it.second->animated);
			}
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			this->onDeserialize.notifyListeners(json);

			for (auto it : this->mappings)
			{
				it.second->animated.set(false);
			}

			if (json.count("Presets"))
			{
				auto & jsonGroup = json["Presets"];
				for (auto it : this->mappings)
				{
					ofxPreset::Serializer::Deserialize(jsonGroup, it.second->animated);
				}
			}

			this->refreshMappings();
		}

		//--------------------------------------------------------------
		string Base::getDataPath(const string & file)
		{
			if (this->dataPath.empty())
			{
				auto tokens = ofSplitString(this->getName(), "::", true, true);
				auto dataPath = ofFilePath::addTrailingSlash(ofToDataPath("../../../../Shared/data"));
				for (auto & component : tokens)
				{
					dataPath = ofFilePath::addTrailingSlash(dataPath.append(component));
				}
				this->dataPath = dataPath;
			}
			if (file.empty()) 
			{
				return this->dataPath;
			}

			auto filePath = this->dataPath;
			filePath.append(file);
			return filePath;
		}

		//--------------------------------------------------------------
		string Base::getPresetPath(const string & preset)
		{
			auto presetPath = ofFilePath::addTrailingSlash(this->getDataPath("presets"));
			if (!preset.empty())
			{
				presetPath.append(ofFilePath::addTrailingSlash(preset));
			}
			return presetPath;
		}

		//--------------------------------------------------------------
		bool Base::loadPreset(const string & presetName)
		{
			const auto presetPath = this->getPresetPath(presetName);
			auto presetFile = ofFile(presetPath);
			if (!presetFile.exists())
			{
				ofLogWarning("Base::loadPreset") << "File not found at path " << presetPath;
				return false;
			}

			auto paramsPath = presetPath;
			paramsPath.append("parameters.json");
			auto paramsFile = ofFile(paramsPath);
			if (paramsFile.exists())
			{
				nlohmann::json json;
				paramsFile >> json;

				this->deserialize(json);
			}

			this->timeline.loadTracksFromFolder(presetPath);

			this->currPreset = presetName;
		}

		//--------------------------------------------------------------
		bool Base::savePreset(const string & presetName)
		{
			const auto presetPath = this->getPresetPath(presetName);

			auto paramsPath = presetPath;
			paramsPath.append("parameters.json");
			auto paramsFile = ofFile(paramsPath, ofFile::WriteOnly);
			nlohmann::json json;
			this->serialize(json);
			paramsFile << json;

			this->timeline.saveTracksToFolder(presetPath);

			this->populatePresets();

			return true;
		}

		//--------------------------------------------------------------
		void Base::populatePresets()
		{
			auto presetsPath = this->getPresetPath();
			auto presetsDir = ofDirectory(presetsPath);
			presetsDir.listDir();
			presetsDir.sort();

			this->presets.clear();
			for (auto i = 0; i < presetsDir.size(); ++i)
			{
				if (presetsDir.getFile(i).isDirectory())
				{
					this->presets.push_back(presetsDir.getName(i));
				}
			}
		}

		//--------------------------------------------------------------
		ofxTimeline & Base::getTimeline()
		{
			return this->timeline;
		}

		//--------------------------------------------------------------
		void Base::populateMappings(const ofParameterGroup & group, string name)
		{
			for (const auto & parameter : group)
			{
				// Group.
				auto parameterGroup = dynamic_pointer_cast<ofParameterGroup>(parameter);
				if (parameterGroup)
				{
					// Recurse through contents.
					this->populateMappings(*parameterGroup, name);
					continue;
				}

				// Parameters.
				auto parameterFloat = dynamic_pointer_cast<ofParameter<float>>(parameter);
				if (parameterFloat)
				{
					auto mapping = make_shared<Mapping<float, ofxTLCurves>>();
					mapping->setup(parameterFloat);
					this->mappings.emplace(mapping->getName(), mapping);
					continue;
				}
				auto parameterInt = dynamic_pointer_cast<ofParameter<int>>(parameter);
				if (parameterInt)
				{
					auto mapping = make_shared<Mapping<int, ofxTLCurves>>();
					mapping->setup(parameterInt);
					this->mappings.emplace(mapping->getName(), mapping);
					continue;
				}
				auto parameterBool = dynamic_pointer_cast<ofParameter<bool>>(parameter);
				if (parameterBool)
				{
					auto mapping = make_shared<Mapping<bool, ofxTLSwitches>>();
					mapping->setup(parameterBool);
					this->mappings.emplace(mapping->getName(), mapping);
					continue;
				}
				auto parameterColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(parameter);
				if (parameterColor)
				{
					auto mapping = make_shared<Mapping<ofFloatColor, ofxTLColorTrack>>();
					mapping->setup(parameterColor);
					this->mappings.emplace(mapping->getName(), mapping);
					continue;
				}
			}
		}

		//--------------------------------------------------------------
		void Base::refreshMappings()
		{
			for (auto & it : this->mappings)
			{
				auto mapping = it.second;
				if (mapping->animated) 
				{
					mapping->addTrack(this->timeline);
				}
				else
				{
					mapping->removeTrack(this->timeline);
				}
			}
		}
	}
}