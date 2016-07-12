#include "Base.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		static const string kPresetDefaultName = "_default";

		//--------------------------------------------------------------
		ofCamera & Base::getCamera()
		{
			return this->camera;
		}
			
		//--------------------------------------------------------------
		Base::Base()
		{}

		//--------------------------------------------------------------
		Base::~Base()
		{}

		//--------------------------------------------------------------
		void Base::setup()
		{
			this->onSetup.notify();
			
			// List presets.
			this->populatePresets();

			// Setup timeline.
			this->timeline.setup();
			this->timeline.setLoopType(OF_LOOP_NONE);
			this->timeline.setFrameRate(30.0f);
			this->timeline.setDurationInSeconds(30);
			this->timeline.setAutosave(false);
			this->timeline.setPageName(this->getParameters().base.getName());

			const auto trackName = "Camera";
			const auto trackIdentifier = this->getParameters().base.getName() + "_" + trackName;
			this->cameraTrack = new ofxTLCameraTrack();
			this->cameraTrack->setCamera(this->getCamera());
			this->cameraTrack->setXMLFileName(this->timeline.nameToXMLName(trackIdentifier));
			this->timeline.addTrack(trackIdentifier, this->cameraTrack);
			this->cameraTrack->setDisplayName(trackName);
			this->cameraTrack->lockCameraToTrack = false;

			// List mappings.
			this->populateMappings(this->getParameters());

			// Load default preset.
			this->currPreset.clear();
			this->loadPreset(kPresetDefaultName);
		}

		//--------------------------------------------------------------
		void Base::exit()
		{
			this->onExit.notify();

			// Save default preset.
			this->savePreset(kPresetDefaultName);
		}

		//--------------------------------------------------------------
		void Base::resize(ofResizeEventArgs & args)
		{
			this->onResize.notify(args);
		}

		//--------------------------------------------------------------
		void Base::update(double dt)
		{
			if (GetApp()->isMouseOverGui())
			{
				this->camera.disableMouseInput();
			}
			else
			{
				this->camera.enableMouseInput();
			}
			
			for (auto & it : this->mappings)
			{
				it.second->update();
			}
			
			this->onUpdate.notify(dt);
		}

		//--------------------------------------------------------------
		void Base::draw()
		{
			this->drawBack();
			this->drawWorld();
			this->drawFront();
		}

		//--------------------------------------------------------------
		void Base::drawBack()
		{
			ofBackground(this->getParameters().base.background.get());

			this->onDrawBack.notify();
		}

		//--------------------------------------------------------------
		void Base::drawWorld()
		{
			this->getCamera().begin();
			{
				this->onDrawWorld.notify();
			}
			this->getCamera().end();
		}

		//--------------------------------------------------------------
		void Base::drawFront()
		{
			this->onDrawFront.notify();
		}

		//--------------------------------------------------------------
		void Base::drawTimeline(ofxPreset::Gui::Settings & settings)
		{
			// Disable mouse events if it's already been captured.
			if (settings.mouseOverGui)
			{
				this->timeline.disableEvents();
			}
			else
			{
				this->timeline.enableEvents();
			}

			this->timeline.setOffset(ofVec2f(0.0, ofGetHeight() - this->timeline.getHeight()));
			this->timeline.draw();
			settings.mouseOverGui |= this->timeline.getDrawRect().inside(ofGetMouseX(), ofGetMouseY());
		}

		//--------------------------------------------------------------
		void Base::gui(ofxPreset::Gui::Settings & settings)
		{
			auto & parameters = this->getParameters();

			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow("Presets", settings))
			{
				if (ImGui::Button("Save"))
				{
					this->savePreset(this->currPreset);
				}
				ImGui::SameLine();
				if (ImGui::Button("Save As..."))
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
				for (auto & it : this->mappings)
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

			if (this->onGuiListeners.size())
			{
				// Add a GUI window for the Base parameters.
				ofxPreset::Gui::SetNextWindow(settings);
				if (ofxPreset::Gui::BeginWindow(parameters.base.getName(), settings))
				{
					ofxPreset::Gui::AddParameter(parameters.base.background);
				}
				ofxPreset::Gui::EndWindow(settings);

				// Let the child class handle its child parameters.
				this->onGui.notify(settings);
			}
			else
			{
				// Build a default GUI for all parameters.
				ofxPreset::Gui::SetNextWindow(settings);
				ofxPreset::Gui::AddGroup(parameters, settings);
			}
		}

		//--------------------------------------------------------------
		void Base::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->getParameters()); 
			ofxPreset::Serializer::Serialize(json, this->getCamera(), "camera");
			
			this->onSerialize.notify(json);

			auto & jsonGroup = json["Mappings"];
			for (auto & it : this->mappings)
			{
				ofxPreset::Serializer::Serialize(jsonGroup, it.second->animated);
			}
		}

		//--------------------------------------------------------------
		void Base::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->getParameters());
			if (json.count("camera"))
			{
				// Disable auto distance so that it doesn't interfere with the camera matrix.
				// This is done here because getCamera() returns an ofCamera and not an ofEasyCam.
				this->camera.setAutoDistance(false);

				ofxPreset::Serializer::Deserialize(json, this->getCamera(), "camera");
			}

			this->onDeserialize.notify(json);

			for (auto & it : this->mappings)
			{
				it.second->animated.set(false);
			}
			if (json.count("Mappings"))
			{
				auto & jsonGroup = json["Mappings"];
				for (auto & it : this->mappings)
				{
					ofxPreset::Serializer::Deserialize(jsonGroup, it.second->animated);
				}
			}
			this->refreshMappings();
		}

        //--------------------------------------------------------------
        string Base::getAssetsPath(const string & file)
        {
            if (this->assetsPath.empty())
            {
                auto tokens = ofSplitString(this->getName(), "::", true, true);
                auto assetsPath = GetSharedAssetsPath();
                for (auto & component : tokens)
                {
                    assetsPath = ofFilePath::addTrailingSlash(assetsPath.append(component));
                }
                this->assetsPath = assetsPath;
            }
            if (file.empty())
            {
                return this->assetsPath;
            }

            auto filePath = this->assetsPath;
            filePath.append(file);
            return filePath;
        }

		//--------------------------------------------------------------
		string Base::getDataPath(const string & file)
		{
			if (this->dataPath.empty())
			{
				auto tokens = ofSplitString(this->getName(), "::", true, true);
				auto dataPath = GetSharedDataPath();
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
		string Base::getCurrentPresetPath(const string & file)
		{
			auto currentPresetPath = this->getPresetPath(this->currPreset);
			if (file.empty())
			{
				return currentPresetPath;
			}

			currentPresetPath.append(file);
			return currentPresetPath;
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

			return true;
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

				// Parameter, try ofParameter first and ofxPreset::Parameter aggregate if that fails.
				{
					auto parameterFloat = dynamic_pointer_cast<ofParameter<float>>(parameter);
					if (!parameterFloat)
					{
						auto aggParameter = dynamic_pointer_cast<ofxPreset::Parameter<float>>(parameter);
						if (aggParameter)
						{
							parameterFloat = aggParameter->getInternal();
						}
					}
					if (parameterFloat)
					{
						auto mapping = make_shared<util::Mapping<float, ofxTLCurves>>();
						mapping->setup(parameterFloat);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterInt = dynamic_pointer_cast<ofParameter<int>>(parameter);
					if (!parameterInt)
					{
						auto aggParameter = dynamic_pointer_cast<ofxPreset::Parameter<int>>(parameter);
						if (aggParameter)
						{
							parameterInt = aggParameter->getInternal();
						}
					}
					if (parameterInt)
					{
						auto mapping = make_shared<util::Mapping<int, ofxTLCurves>>();
						mapping->setup(parameterInt);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterBool = dynamic_pointer_cast<ofParameter<bool>>(parameter);
					if (!parameterBool)
					{
						auto aggParameter = dynamic_pointer_cast<ofxPreset::Parameter<bool>>(parameter);
						if (aggParameter)
						{
							parameterBool = aggParameter->getInternal();
						}
					}
					if (parameterBool)
					{
						auto mapping = make_shared<util::Mapping<bool, ofxTLSwitches>>();
						mapping->setup(parameterBool);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(parameter);
					if (!parameterColor)
					{
						auto aggParameter = dynamic_pointer_cast<ofxPreset::Parameter<ofFloatColor>>(parameter);
						if (aggParameter)
						{
							parameterColor = aggParameter->getInternal();
						}
					}
					if (parameterColor)
					{
						auto mapping = make_shared<util::Mapping<ofFloatColor, ofxTLColorTrack>>();
						mapping->setup(parameterColor);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}
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

		//--------------------------------------------------------------
		void Base::setCameraLocked(bool cameraLocked)
		{
			this->cameraTrack->lockCameraToTrack = cameraLocked;
		}

		//--------------------------------------------------------------
		void Base::toggleCameraLocked()
		{
			this->cameraTrack->lockCameraToTrack ^= 1;
		}

		//--------------------------------------------------------------
		bool Base::isCameraLocked() const
		{
			return this->cameraTrack->lockCameraToTrack;
		}

		//--------------------------------------------------------------
		void Base::addCameraKeyframe()
		{
			this->cameraTrack->addKeyframe();
		}
	}
}