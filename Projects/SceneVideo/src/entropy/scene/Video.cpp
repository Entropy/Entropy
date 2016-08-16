#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Video::Video()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		Video::~Video()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Video::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			this->dirtyBounds = false;
		}
		
		//--------------------------------------------------------------
		void Video::exit()
		{

		}

		//--------------------------------------------------------------
		void Video::resize(ofResizeEventArgs & args)
		{
			this->dirtyBounds = true;
		}

		//--------------------------------------------------------------
		void Video::update(double & dt)
		{
			if (this->dirtyBounds)
			{
				const auto contentMode = static_cast<ContentMode>(this->parameters.contentMode.get());
				if (contentMode == ContentMode::Center)
				{
					this->drawBounds.setFromCenter(GetCanvasWidth() * 0.5f, GetCanvasHeight() * 0.5f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
				}
				else if (contentMode == ContentMode::TopLeft)
				{
					this->drawBounds.set(0.0f, 0.0f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
				}
				else if (contentMode == ContentMode::ScaleToFill)
				{
					this->drawBounds.set(0.0f, 0.0f, GetCanvasWidth(), GetCanvasHeight());
				}
				else
				{
					const auto canvasRatio = GetCanvasWidth() / GetCanvasHeight();
					const auto videoRatio = this->videoPlayer.getWidth() / this->videoPlayer.getHeight();

					if (!(canvasRatio > videoRatio) ^ !(contentMode == ContentMode::ScaleAspectFit))
					{
						this->drawBounds.width = GetCanvasWidth();
						this->drawBounds.height = this->drawBounds.width / videoRatio;
						this->drawBounds.x = 0.0f;
						this->drawBounds.y = (GetCanvasHeight() - this->drawBounds.height) * 0.5f;
					}
					else
					{
						this->drawBounds.height = GetCanvasHeight();
						this->drawBounds.width = this->drawBounds.height * videoRatio;
						this->drawBounds.y = 0.0f;
						this->drawBounds.x = (GetCanvasWidth() - this->drawBounds.width) * 0.5f;
					}
				}

				this->dirtyBounds = false;
			}

			this->videoPlayer.update();
			if (this->videoPlayer.isStopped() && this->parameters.playback.play)
			{
				this->parameters.playback.play.set(false);
			}
		}

		//--------------------------------------------------------------
		void Video::drawBack()
		{
			if (this->videoPlayer.isLoaded())
			{
				this->videoPlayer.draw(this->drawBounds.x, this->drawBounds.y, this->drawBounds.width, this->drawBounds.height);
			}
		}

		//--------------------------------------------------------------
		void Video::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName(), settings))
			{
				if (ImGui::Button("Load File..."))
				{
					auto dialogResult = ofSystemLoadDialog("Load File", false, this->getAssetsPath("videos"));
					if (dialogResult.bSuccess)
					{
						this->loadVideo(dialogResult.filePath);
					}
				}
				if (!this->parameters.videoPath.get().empty())
				{
					ImGui::Text("File: %s", this->fileName.c_str());
				}

				static vector<string> contentModes;
				if (contentModes.empty())
				{
					contentModes.push_back("Center");
					contentModes.push_back("Top Left");
					contentModes.push_back("Scale To Fill");
					contentModes.push_back("Scale Aspect Fill");
					contentModes.push_back("Scale Aspect Fit");
				}
				if (ImGui::Button("Content Mode..."))
				{
					ImGui::OpenPopup("Content Modes");
					ImGui::SameLine();
				}
				if (ImGui::BeginPopup("Content Modes"))
				{
					for (auto i = 0; i < contentModes.size(); ++i)
					{
						if (ImGui::Selectable(contentModes[i].c_str()))
						{
							this->parameters.contentMode = i;
							this->dirtyBounds = true;
						}
					}
					ImGui::EndPopup();
				}
				ImGui::SameLine();
				ImGui::Text(contentModes[this->parameters.contentMode].c_str());

				if (ImGui::CollapsingHeader(this->parameters.playback.getName().c_str(), nullptr, true, true))
				{
					if (ofxPreset::Gui::AddParameter(this->parameters.playback.play))
					{
						if (this->parameters.playback.play)
						{
							this->videoPlayer.play();
						}
						else
						{
							this->videoPlayer.stop();
						}
					}

					if (ofxPreset::Gui::AddParameter(this->parameters.playback.loop))
					{
						if (this->parameters.playback.loop)
						{
							this->videoPlayer.setLoopState(OF_LOOP_NORMAL);
						}
						else
						{
							this->videoPlayer.setLoopState(OF_LOOP_NONE);
						}
					}
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Video::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		void Video::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.videoPath.get().empty())
			{
				this->loadVideo(ofFilePath::addTrailingSlash(this->getAssetsPath("videos")).append(this->parameters.videoPath));
			}
		}

		//--------------------------------------------------------------
		bool Video::loadVideo(const string & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No video found at " << filePath;
				return false;
			}
			
			this->videoPlayer.loadAsync(filePath);
			
			if (this->parameters.playback.play)
			{
				this->videoPlayer.play();
			}

			if (this->parameters.playback.loop)
			{
				this->videoPlayer.setLoopState(OF_LOOP_NORMAL);
			}
			else
			{
				this->videoPlayer.setLoopState(OF_LOOP_NONE);
			}

			this->parameters.videoPath = ofFilePath::makeRelative(this->getAssetsPath("videos"), filePath);
			this->dirtyBounds = true;

			ofFile file = ofFile(filePath);
			this->fileName = file.getFileName();

			return true;
		}
	}
}