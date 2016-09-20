#include "Playlist.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Playlist::Playlist()
			: currentTrack(-1)
		{
			this->loadSettings();
		}

		//--------------------------------------------------------------
		Playlist::~Playlist()
		{
			// Clear scenes.
			this->unsetCurrent();
			this->scenes.clear();
		}

		//--------------------------------------------------------------
		bool Playlist::addScene(shared_ptr<Base> scene)
		{
			if (this->scenes.find(scene->getName()) == this->scenes.end())
			{
				ofLogNotice(__FUNCTION__) << "Initializing scene " << scene->getName() << "...";
				auto startTime = ofGetElapsedTimef();

				scene->init_();
				this->scenes.emplace(scene->getName(), scene);
				
				// Get the short name for the gui.
				auto tokens = ofSplitString(scene->getName(), "::", true, true);
				this->shortNames.emplace(tokens.back(), scene->getName());

				auto duration = ofGetElapsedTimef() - startTime;
				ofLogNotice(__FUNCTION__) << "Scene " << scene->getName() << " initialized in " << duration << " seconds.";
				
				return true;
			}
			
			ofLogError(__FUNCTION__) << "Scene with name " << scene->getName() << " already exists!";
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::removeScene(shared_ptr<Base> scene)
		{
			return this->removeScene(scene->getName());
		}

		//--------------------------------------------------------------
		bool Playlist::removeScene(const string & name)
		{
			// scene->clear_() will be called by the destructor.
			
			if (this->scenes.erase(name))
			{				
				string foundKey;
				for (auto & it : this->shortNames)
				{
					if (it.second == name)
					{
						foundKey = it.first;
						break;
					}
				}
				if (!foundKey.empty())
				{
					this->shortNames.erase(foundKey);
				}

				return true;
			}

			ofLogError(__FUNCTION__) << "Scene with name " << name << " does not exist!";
			return false;
		}

		//--------------------------------------------------------------
		void Playlist::previewScene()
		{
			// Just start the first scene in the map.
			if (this->setCurrentScene(this->scenes.begin()->first))
			{
				this->setCurrentPreset(kPresetDefaultName, false);
			}
		}

		//--------------------------------------------------------------
		shared_ptr<Base> Playlist::getScene(const string & name)
		{
			try
			{
				return this->scenes.at(name);
			}
			catch (const out_of_range &)
			{
				ofLogError(__FUNCTION__) << "Scene with name " << name << " does not exist!";
				return nullptr;
			}
		}

		//--------------------------------------------------------------
		template<typename SceneType>
		shared_ptr<SceneType> Playlist::getScene(const string & name)
		{
			auto scene = this->getScene(name);
			if (scene)
			{
				auto sceneTyped = dynamic_pointer_cast<SceneType>(scene);
				if (sceneTyped)
				{
					return sceneTyped;
				}
				ofLogError(__FUNCTION__) << "Scene with name " << name << " does not match template!";
			}
			return nullptr;
		}

		//--------------------------------------------------------------
		shared_ptr<Base> Playlist::getCurrentScene() const
		{
			return this->currentScene;
		}

		//--------------------------------------------------------------
		template<typename SceneType>
		shared_ptr<SceneType> Playlist::getCurrentScene()
		{
			if (this->currentScene)
			{
				auto sceneTyped = dynamic_pointer_cast<SceneType>(this->currentScene);
				if (sceneTyped)
				{
					return sceneTyped;
				}
				ofLogError(__FUNCTION__) << "Current scene does not match template!";
			}
			return nullptr;
		}

		//--------------------------------------------------------------
		bool Playlist::setCurrentScene(const string & name)
		{
			if (this->currentScene && this->currentScene->getName() == name)
			{
				ofLogNotice(__FUNCTION__) << "Scene " << name << " already set.";
				return true;
			}

			this->unsetCurrent();
			
			auto scene = this->getScene(name);
			if (scene)
			{
				this->currentScene = scene;

				this->presetCuedListener = this->currentScene->presetCuedEvent.newListener([this](string & preset)
				{
					this->nextPreset = preset;
				});
				this->presetSavedListener = this->currentScene->presetSavedEvent.newListener([this](string & preset)
				{
					this->cameraSettings[render::Layout::Back] = this->currentScene->getCamera(render::Layout::Back)->fetchSettings();
					this->cameraSettings[render::Layout::Front] = this->currentScene->getCamera(render::Layout::Front)->fetchSettings();
				});

				for (auto & it : this->cameraControlAreas)
				{
					this->currentScene->setCameraControlArea(it.first, it.second);
				}

				return true;
			}
			ofLogError(__FUNCTION__) << "Scene with name " << name << " does not exist!";
			return false;
		}

		//--------------------------------------------------------------
		void Playlist::unsetCurrent()
		{
			if (this->currentScene)
			{
				this->currentScene->exit_();
			}
			this->currentScene.reset();

			this->presetCuedListener.unsubscribe();
			this->presetSavedListener.unsubscribe();
			this->nextPreset.clear();
		}

		//--------------------------------------------------------------
		const string & Playlist::getCurrentPresetName() const
		{
			if (this->currentScene)
			{
				return this->currentScene->getCurrentPresetName();
			}

			static string dummyStr;
			return dummyStr;
		}

		//--------------------------------------------------------------
		bool Playlist::setCurrentPreset(const string & name, bool showtime)
		{
			if (this->currentScene)
			{
				if (this->currentScene->loadPreset(name))
				{
					if (showtime)
					{
						this->currentScene->setShowtime();
					}
					return true;
				}
			}
			return false;
		}

		//--------------------------------------------------------------
		void Playlist::addTrack(const string & sceneName, const string & presetName)
		{
			this->tracks.push_back(make_pair(sceneName, presetName));
		}
		
		//--------------------------------------------------------------
		void Playlist::removeTrack()
		{
			if (!this->tracks.empty())
			{
				if (this->currentTrack == this->tracks.size() - 1)
				{
					// We're currently playing the track to remove, unplay it.
					this->unsetCurrent();
				}
				
				this->tracks.pop_back();
			}
		}

		//--------------------------------------------------------------
		bool Playlist::playTrack(size_t index)
		{
			if (index >= this->tracks.size())
			{
				ofLogError(__FUNCTION__) << "Index " << index << " out of range!";
				return false;
			}

			auto & item = this->tracks[index];
			if (this->shortNames.find(item.first) != this->shortNames.end())
			{
				auto & sceneName = this->shortNames[item.first];
				if (this->setCurrentScene(sceneName))
				{
					auto found = this->setCurrentPreset(item.second, true);
					if (!found)
					{
						ofLogWarning(__FUNCTION__) << "Preset " << item.second << " not found, reverting to default.";
						item.second = kPresetDefaultName;
						this->setCurrentPreset(item.second, true);
					}
					this->currentTrack = index;

					this->presetLoadedListener = this->currentScene->presetLoadedEvent.newListener([this](string & preset)
					{
						this->tracks[this->currentTrack].second = preset;
					});

					return found;
				}
			}

			ofLogError(__FUNCTION__) << "Scene " << item.first << " not loaded!";
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::stopTrack()
		{
			if (this->currentTrack >= this->tracks.size())
			{
				ofLogError(__FUNCTION__) << "No track playing!";
				return false;
			}

			this->unsetCurrent();
			this->presetLoadedListener.unsubscribe();
			this->currentTrack = -1;
			return true;
		}

		//--------------------------------------------------------------
		size_t Playlist::getCurrentTrack() const
		{
			return this->currentTrack;
		}

		//--------------------------------------------------------------
		bool Playlist::update(double dt)
		{
			if (this->currentScene)
			{
				if (!this->nextPreset.empty())
				{
					this->setCurrentPreset(this->nextPreset, false);
					this->nextPreset.clear();
					return true;
				}
				
				this->currentScene->update_(dt);
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::drawSceneBase(render::Layout layout)
		{
			if (this->currentScene)
			{
				this->currentScene->drawBase_(layout);
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::drawSceneWorld(render::Layout layout)
		{
			if (this->currentScene)
			{
				this->currentScene->drawWorld_(layout);
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::drawSceneOverlay(render::Layout layout)
		{
			if (this->currentScene)
			{
				this->currentScene->drawOverlay_(layout);
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::drawGui(ofxPreset::Gui::Settings & settings)
		{
			if (this->scenes.size() > 1)
			{
				ofxPreset::Gui::SetNextWindow(settings);
				if (ofxPreset::Gui::BeginWindow("Playlist", settings))
				{
					if (ImGui::Button("Save"))
					{
						this->saveSettings();
					}
					ImGui::SameLine();
					if (ImGui::Button("Load"))
					{
						this->loadSettings();
					}

					if (ImGui::Button("Add Track..."))
					{
						ImGui::OpenPopup("Tracks");
						ImGui::SameLine();
					}
					if (ImGui::BeginPopup("Tracks"))
					{
						for (auto & it : this->shortNames)
						{
							if (ImGui::Selectable(it.first.c_str()))
							{
								this->addTrack(it.first, kPresetDefaultName);
							}
						}
						ImGui::EndPopup();
					}
					if (!this->tracks.empty())
					{
						ImGui::SameLine();
						if (ImGui::Button("Remove Track"))
						{
							this->removeTrack();
						}
					}

					if (ofxPreset::Gui::BeginTree("Tracks", settings))
					{
						for (int i = 0; i < this->tracks.size(); ++i)
						{
							const auto savedCurrTrack = this->currentTrack;
							if (i == savedCurrTrack)
							{
								ImGui::PushStyleColor(ImGuiCol_Text, ImColor(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]));
							}
							ImGui::PushID(i);
							ImGui::AlignFirstTextHeightToWidgets();
							ImGui::Text("%d", i);
							ImGui::SameLine(40);
							ImGui::Text(this->tracks[i].first.c_str());
							ImGui::SameLine(120);
							ImGui::AlignFirstTextHeightToWidgets();
							ImGui::Text(this->tracks[i].second.c_str());
							ImGui::SameLine(200);
							ImGui::PushItemWidth(-1);
							if (i == savedCurrTrack)
							{
								ImGui::PopStyleColor();
							}
							if (i == savedCurrTrack)
							{
								if (ImGui::Button("Stop"))
								{
									this->stopTrack();
								}
							}
							else
							{
								if (ImGui::Button("Play"))
								{
									this->playTrack(i);
								}
							}
							ImGui::PopItemWidth();
							ImGui::PopID();
						}

						ofxPreset::Gui::EndTree(settings);
					}
				}
				ofxPreset::Gui::EndWindow(settings);
			}
			
			if (this->currentScene)
			{
				if (this->scenes.size() > 1)
				{
					// Move to the next column for the Scene specific gui windows.
					//settings.windowPos = glm::vec2(settings.totalBounds.x + kGuiMargin, 0.0f);
					settings.windowPos = glm::vec2(400.0f + kGuiMargin, 0.0f);
					settings.windowSize = glm::vec2(0.0f);
				}

				this->currentScene->gui_(settings);
			}

			return true;
		}

		//--------------------------------------------------------------
		bool Playlist::drawTimeline(ofxPreset::Gui::Settings & settings)
		{
			if (this->currentScene)
			{
				this->currentScene->drawTimeline(settings);
				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::postProcess(render::Layout layout, const ofTexture & srcTexture, const ofFbo & dstFbo) const
		{
			if (this->currentScene)
			{
				if (layout == render::Layout::Back)
				{
					return this->currentScene->postProcessBack(srcTexture, dstFbo);
				}
				return this->currentScene->postProcessFront(srcTexture, dstFbo);
			}
			return false;
		}

		//--------------------------------------------------------------
		bool Playlist::keyPressed(ofKeyEventArgs & args)
		{
			if (this->currentScene)
			{
				if (args.key == 'L')
				{
					this->currentScene->toggleCameraLocked();
					return true;
				}
				if (args.key == 'B')
				{
					this->currentScene->addCameraKeyframe(render::Layout::Back);
					return true;
				}
				if (args.key == 'F')
				{
					this->currentScene->addCameraKeyframe(render::Layout::Front);
					return true;
				}
				if (args.key == ' ')
				{
					this->currentScene->goToNextTimelineFlag();
					return true;
				}
			}
			return false;
		}

		//--------------------------------------------------------------
		void Playlist::setCameraControlArea(render::Layout layout, const ofRectangle & controlArea)
		{
			this->cameraControlAreas[layout] = controlArea;
			
			if (this->currentScene)
			{
				this->currentScene->setCameraControlArea(layout, controlArea);
			}
		}

		//--------------------------------------------------------------
		const world::Camera::Settings & Playlist::getCameraSettings(render::Layout layout)
		{
			return this->cameraSettings[layout];
		}

		//--------------------------------------------------------------
		void Playlist::canvasResized(render::Layout layout, ofResizeEventArgs & args)
		{
			for (auto & it : this->scenes)
			{
				it.second->resize_(layout, args);
			}
		}

		//--------------------------------------------------------------
		const string & Playlist::getDataPath()
		{
			static string dataPath;
			if (dataPath.empty())
			{
				dataPath = GetSharedDataPath();
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("entropy"));
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("scene"));
				dataPath = ofFilePath::addTrailingSlash(dataPath.append("Playlist"));
			}
			return dataPath;
		}

		//--------------------------------------------------------------
		const string & Playlist::getSettingsFilePath()
		{
			static string filePath;
			if (filePath.empty())
			{
				filePath = this->getDataPath();
				filePath.append("settings.json");
			}
			return filePath;
		}

		//--------------------------------------------------------------
		bool Playlist::loadSettings()
		{
			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::ReadOnly);
			if (!file.exists())
			{
				ofLogWarning(__FUNCTION__) << "File not found at path " << filePath;
				return false;
			}

			nlohmann::json json;
			file >> json;

			this->deserialize(json);

			return true;
		}

		//--------------------------------------------------------------
		bool Playlist::saveSettings()
		{
			nlohmann::json json;
			this->serialize(json);

			auto filePath = this->getSettingsFilePath();
			auto file = ofFile(filePath, ofFile::WriteOnly);
			file << json.dump(4);

			return true;
		}

		//--------------------------------------------------------------
		void Playlist::serialize(nlohmann::json & json)
		{
			// Serialize the tracks.
			vector<nlohmann::json> jsonQueue;
			for (auto & pair : this->tracks)
			{
				nlohmann::json jsonPair;
				jsonPair["scene"] = pair.first;
				jsonPair["preset"] = pair.second;
				jsonQueue.push_back(jsonPair);
			}
			json["tracks"] = jsonQueue;
		}

		//--------------------------------------------------------------
		void Playlist::deserialize(const nlohmann::json & json)
		{
			// Deserialize the tracks.
			this->tracks.clear();
			for (auto & jsonPair : json["tracks"])
			{
				string scene = jsonPair["scene"];
				string preset = jsonPair["preset"];
				this->tracks.push_back(make_pair(scene, preset));
			}
		}
	}
}