#include "Playlist.h"

#include "entropy/Helpers.h"
#include "entropy/scene/Calibrate.h"
#include "entropy/scene/Empty.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Playlist::Playlist()
			: currentTrack(-1)
			, currentScene(nullptr)
		{
		}

		//--------------------------------------------------------------
		Playlist::~Playlist()
		{
			this->stopTrack();
		}

		//--------------------------------------------------------------
		bool Playlist::isActive() const
		{
			return (this->currentTrack < this->tracks.size());
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
		bool Playlist::addTrack(const string & sceneName, const string & presetName)
		{
			std::shared_ptr<Base> scene;
			if (sceneName == "Empty")
			{
				scene = std::make_shared<Empty>();
			}
			else if (sceneName == "Calibrate")
			{
				scene = std::make_shared<Calibrate>();
			}
			else
			{
				ofLogError(__FUNCTION__) << "Scene " << sceneName << " not found!";
				return false;
			}

			ofLogNotice(__FUNCTION__) << "Adding track for Scene " << sceneName;

			// Init the scene.
			auto elapsedTime = ofGetElapsedTimef();
			scene->init_();
			ofLogNotice(__FUNCTION__) << "Scene " << sceneName << " loaded in " << (ofGetElapsedTimef() - elapsedTime) << " seconds";

			//// Load the matching preset.
			//elapsedTime = ofGetElapsedTimef(); 
			//if (scene->loadPreset(presetName))
			//{
			//	ofLogNotice(__FUNCTION__) << "Preset " << presetName << " loaded in " << (ofGetElapsedTimef() - elapsedTime) << " seconds";
			//}
			//else
			//{
			//	ofLogError(__FUNCTION__) << "Preset " << presetName << " not found for scene " << sceneName << "!";
			//	return false;
			//}

			this->tracks.push_back(std::make_pair(scene, presetName));
		}
		
		//--------------------------------------------------------------
		bool Playlist::removeTrack()
		{
			if (!this->tracks.empty())
			{
				if (this->currentTrack == this->tracks.size() - 1)
				{
					// We're currently playing the track to remove, unplay it.
					this->stopTrack();
				}
				
				this->tracks.pop_back();

				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		void Playlist::preloadTracks()
		{
			for (auto track : this->tracks)
			{
				// Load the preset per track.
				auto elapsedTime = ofGetElapsedTimef();
				if (track.first->loadPreset(track.second))
				{
					ofLogNotice(__FUNCTION__) << "Preset " << track.second << " loaded in " << (ofGetElapsedTimef() - elapsedTime) << " seconds";
				}
				else
				{
					ofLogError(__FUNCTION__) << "Preset " << track.second << " not found for scene " << track.first->getShortName() << "!";
				}
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

			if (index == this->currentTrack)
			{
				ofLogWarning(__FUNCTION__) << "Index " << index << " is already playing!";
				return false;
			}

			this->currentTrack = index;
			this->currentScene = this->tracks[this->currentTrack].first;

			if (!this->currentScene->isReady())
			{
				// Load the scene before moving on.
				this->currentScene->loadPreset(this->tracks[this->currentTrack].second);
			}
			else
			{
				this->currentScene->refresh_();
			}

			// Add preset listeners.
			this->presetLoadedListener = this->currentScene->presetLoadedEvent.newListener([this](string & preset)
			{
				this->tracks[this->currentTrack].second = preset;
			});
			this->presetCuedListener = this->currentScene->presetCuedEvent.newListener([this](string & preset)
			{
				this->nextPreset = preset;
			});
			this->presetSavedListener = this->currentScene->presetSavedEvent.newListener([this](string & preset)
			{
				this->cameraSettings[render::Layout::Back] = this->currentScene->getCamera(render::Layout::Back)->fetchSettings();
				this->cameraSettings[render::Layout::Front] = this->currentScene->getCamera(render::Layout::Front)->fetchSettings();
			});

			// Set the camera control areas for the scene.
			for (auto & it : this->cameraControlAreas)
			{
				this->currentScene->setCameraControlArea(it.first, it.second);
			}

			// Start the show.
			this->currentScene->setShowtime();

			return true;
		}

		//--------------------------------------------------------------
		bool Playlist::stopTrack()
		{
			if (this->currentTrack >= this->tracks.size())
			{
				ofLogError(__FUNCTION__) << "No track playing!";
				return false;
			}

			this->currentScene->exit_();
			
			this->presetLoadedListener.unsubscribe();
			this->presetCuedListener.unsubscribe();
			this->presetSavedListener.unsubscribe();
			this->nextPreset.clear();

			this->currentTrack = -1;
			this->currentScene.reset();
			
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
			if (this->currentTrack < this->tracks.size())
			{
				auto scene = this->tracks[this->currentTrack].first;
				
				if (!this->nextPreset.empty())
				{
					scene->loadPreset(this->nextPreset);
					this->nextPreset.clear();
					return true;
				}
				
				scene->update_(dt);
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
		bool Playlist::drawGui(ofxImGui::Settings & settings)
		{
			ofxImGui::SetNextWindow(settings);
			if (ofxImGui::BeginWindow("Playlist", settings))
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
					static const std::vector<std::string> sceneNames{ "Calibrate", "Empty" };
					for (auto & it : sceneNames)
					{
						if (ImGui::Selectable(it.c_str()))
						{
							this->addTrack(it, kPresetDefaultName);
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

				if (ofxImGui::BeginTree("Tracks", settings))
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
						ImGui::Text(this->tracks[i].first->getShortName().c_str());
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

					ofxImGui::EndTree(settings);
				}

				if (ImGui::Button("Preload Tracks"))
				{
					this->preloadTracks();
				}
			}
			ofxImGui::EndWindow(settings);
			
			if (this->currentScene != nullptr)
			{
				// Move to the next column for the Scene specific gui windows.
				//settings.windowPos = glm::vec2(settings.totalBounds.x + kImGuiMargin, 0.0f);
				settings.windowPos = glm::vec2(400.0f + kImGuiMargin, 0.0f);
				settings.windowSize = glm::vec2(0.0f);

				this->currentScene->gui_(settings);
			}

			return true;
		}

		//--------------------------------------------------------------
		bool Playlist::drawTimeline(ofxImGui::Settings & settings)
		{
			if (this->currentScene)
			{
				this->currentScene->drawTimeline(settings);
				return true;
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
			for (auto & it : this->tracks)
			{
				it.first->resize_(layout, args);
			}
		}

		//--------------------------------------------------------------
		const std::filesystem::path & Playlist::getDataPath()
		{
			static std::filesystem::path dataPath;
			if (dataPath.empty())
			{
				dataPath = GetSharedDataPath() / "entropy" / "scene" / "Playlist";
			}
			return dataPath;
		}

		//--------------------------------------------------------------
		const std::filesystem::path & Playlist::getSettingsFilePath()
		{
			static std::filesystem::path filePath;
			if (filePath.empty())
			{
				filePath = this->getDataPath() / "settings.json";
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
			std::vector<nlohmann::json> jsonQueue;
			for (auto & pair : this->tracks)
			{
				nlohmann::json jsonPair;
				jsonPair["scene"] = pair.first->getName();
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
				std::string scene = jsonPair["scene"];
				std::string preset = jsonPair["preset"];

				this->addTrack(scene, preset);
			}
		}
	}
}