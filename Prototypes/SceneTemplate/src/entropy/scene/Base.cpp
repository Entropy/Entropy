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
			this->refreshPresetsList();
			this->loadPreset(kPresetDefaultName);
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
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			this->onDeserialize.notifyListeners(json);
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
				presetPath.append(preset);
			}
			return presetPath;
		}

		//--------------------------------------------------------------
		bool Base::loadPreset(const string & presetName)
		{
			auto filePath = this->getPresetPath(presetName);
			filePath = ofFilePath::addTrailingSlash(filePath);
			filePath.append("parameters.json");

			auto file = ofFile(filePath);
			if (file.exists())
			{
				nlohmann::json json;
				file >> json;

				this->deserialize(json);

				this->currPreset = presetName;

				return true;
			}

			ofLogWarning("Base::loadPreset") << "File not found at path " << filePath;
			return false;
		}

		//--------------------------------------------------------------
		bool Base::savePreset(const string & presetName)
		{
			nlohmann::json json;

			this->serialize(json);

			auto filePath = this->getPresetPath(presetName);
			filePath = ofFilePath::addTrailingSlash(filePath);
			filePath.append("parameters.json");

			auto file = ofFile(filePath, ofFile::WriteOnly);
			file << json;

			this->refreshPresetsList();

			return true;
		}

		//--------------------------------------------------------------
		void Base::refreshPresetsList()
		{
			auto presetsPath = this->getPresetPath();
			auto presetsDir = ofDirectory(presetsPath);
			presetsDir.listDir();
			this->presets.clear();
			for (auto i = 0; i < presetsDir.size(); ++i)
			{
				if (presetsDir.getFile(i).isDirectory())
				{
					this->presets.push_back(presetsDir.getName(i));
				}
			}
		}
	}
}