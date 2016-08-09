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
		void Base::setup_()
		{
			// Set data path root for scene.
			ofSetDataPathRoot(this->getDataPath());
			
			// Setup default camera.
			this->camera.setupPerspective(false, 60.0f, 0.1f, 2000.0f);
			this->camera.setAspectRatio(GetCanvasWidth() / GetCanvasHeight());
			this->camera.setNearClip(0.1);
			this->camera.setFarClip(100000);

			// Setup child Scene.
			this->onSetup.notify();

			// List presets.
			this->populatePresets();

			// Setup timeline.
			static string timelineDataPath;
			if (timelineDataPath.empty())
			{
				timelineDataPath = ofFilePath::addTrailingSlash(GetSharedDataPath());
				timelineDataPath.append(ofFilePath::addTrailingSlash("ofxTimeline"));
			}
			this->timeline.setup(timelineDataPath);
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
		void Base::exit_()
		{
			this->onExit.notify();

			// Save default preset.
			this->savePreset(kPresetDefaultName);
		}

		//--------------------------------------------------------------
		void Base::resize_(ofResizeEventArgs & args)
		{
			this->getCamera().setAspectRatio(args.width / static_cast<float>(args.height));

			this->onResize.notify(args);
		}

		//--------------------------------------------------------------
		void Base::update_(double dt)
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

			for (auto popUp : this->popUps)
			{
				popUp->update(dt);
			}

			this->onUpdate.notify(dt);
		}

		//--------------------------------------------------------------
		void Base::drawBack_()
		{
			ofBackground(this->getParameters().base.background.get());

			this->onDrawBack.notify();
		}

		//--------------------------------------------------------------
		void Base::drawWorld_()
		{
			this->getCamera().begin(GetCanvasViewport());
			ofEnableDepthTest();
			{
				this->onDrawWorld.notify();
			}
			ofDisableDepthTest();
			this->getCamera().end();
		}

		//--------------------------------------------------------------
		void Base::drawFront_()
		{
			this->onDrawFront.notify();

			for (auto popUp : this->popUps)
			{
				popUp->draw();
			}
		}

		//--------------------------------------------------------------
		bool Base::postProcess(const ofTexture & srcTexture, const ofFbo & dstFbo)
		{
			// No post-processing by default, override and return true if required.
			return false;
		}

		//--------------------------------------------------------------
		void Base::gui_(ofxPreset::Gui::Settings & settings)
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
			if (ofxPreset::Gui::BeginWindow("Pop-Ups", settings))
			{
				ImGui::ListBoxHeader("List", 3);
				for (auto i = 0; i < this->popUps.size(); ++i)
				{
					auto name = "Pop-Up " + ofToString(i);
					ImGui::Checkbox(name.c_str(), &this->popUps[i]->editing);
				}
				ImGui::ListBoxFooter();

				if (ImGui::Button("Add Pop-Up..."))
				{
					ImGui::OpenPopup("Pop-Ups");
					ImGui::SameLine();
				}
				if (ImGui::BeginPopup("Pop-Ups"))
				{
					static vector<string> popUpNames;
					if (popUpNames.empty())
					{
						popUpNames.push_back("Image");
					}
					for (auto i = 0; i < popUpNames.size(); ++i)
					{
						if (ImGui::Selectable(popUpNames[i].c_str()))
						{
							if (i == 0)
							{
								this->addPopUp(popup::Base::Type::Image);
							}
						}
					}
					ImGui::EndPopup();
				}

				if (!this->popUps.empty())
				{
					ImGui::SameLine();
					if (ImGui::Button("Remove Pop-Up"))
					{
						this->removePopUp();
					}
				}
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

			// Pop-up gui windows.
			{
				auto popUpSettings = ofxPreset::Gui::Settings();
				popUpSettings.windowPos.x += (settings.windowSize.x + kGuiMargin) * 2.0f;
				popUpSettings.windowPos.y = 0.0f;
				for (auto i = 0; i < this->popUps.size(); ++i)
				{
					this->popUps[i]->gui(popUpSettings);
				}
				settings.mouseOverGui |= popUpSettings.mouseOverGui;
			}

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
		void Base::serialize_(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->getParameters());
			ofxPreset::Serializer::Serialize(json, this->getCamera(), "Camera");

			this->onSerialize.notify(json);

			auto & jsonMappings = json["Mappings"];
			for (auto & it : this->mappings)
			{
				ofxPreset::Serializer::Serialize(jsonMappings, it.second->animated);
			}

			auto & jsonPopUps = json["Pop-Ups"];
			for (auto popUp : this->popUps)
			{
				nlohmann::json jsonPopUp;
				popUp->serialize(jsonPopUp);
				jsonPopUps.push_back(jsonPopUp);
			}
		}

		//--------------------------------------------------------------
		void Base::deserialize_(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->getParameters());
			if (json.count("Camera"))
			{
				// Disable auto distance so that it doesn't interfere with the camera matrix.
				// This is done here because getCamera() returns an ofCamera and not an ofEasyCam.
				this->camera.setAutoDistance(false);

				ofxPreset::Serializer::Deserialize(json, this->getCamera(), "Camera");
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

			if (json.count("Pop-Ups"))
			{
				this->popUps.clear();
				for (auto & jsonPopUp : json["Pop-Ups"])
				{
					int typeAsInt = jsonPopUp["type"];
					popup::Base::Type type = static_cast<popup::Base::Type>(typeAsInt);

					auto popUp = this->addPopUp(type);
					if (popUp)
					{
						popUp->deserialize(jsonPopUp);
					}
				}
			}
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

				this->deserialize_(json);
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
			this->serialize_(json);
			paramsFile << json.dump(4);

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
					if (parameterFloat)
					{
						auto mapping = make_shared<util::Mapping<float, ofxTLCurves>>();
						mapping->setup(parameterFloat);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterInt = dynamic_pointer_cast<ofParameter<int>>(parameter);
					if (parameterInt)
					{
						auto mapping = make_shared<util::Mapping<int, ofxTLCurves>>();
						mapping->setup(parameterInt);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterBool = dynamic_pointer_cast<ofParameter<bool>>(parameter);
					if (parameterBool)
					{
						auto mapping = make_shared<util::Mapping<bool, ofxTLSwitches>>();
						mapping->setup(parameterBool);
						this->mappings.emplace(mapping->getName(), mapping);
						continue;
					}

					auto parameterColor = dynamic_pointer_cast<ofParameter<ofFloatColor>>(parameter);
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
		shared_ptr<popup::Base> Base::addPopUp(popup::Base::Type type)
		{
			shared_ptr<popup::Base> popUp;
			if (type == popup::Base::Type::Image)
			{
				popUp = make_shared<popup::Image>();
			}
			else
			{
				ofLogError(__FUNCTION__) << "Unrecognized pop-up type " << static_cast<int>(type);
				return nullptr;
			}

			auto idx = this->popUps.size();
			popUp->setup(idx);
			popUp->addTrack(this->timeline);
			this->popUps.push_back(popUp);

			return popUp;
		}

		//--------------------------------------------------------------
		void Base::removePopUp()
		{
			this->popUps.pop_back();
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