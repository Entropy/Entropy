#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Video::Video()
		{}
		
		//--------------------------------------------------------------
		Video::~Video()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Video::setup()
		{
			this->dirtyBounds = false;
		}
		
		//--------------------------------------------------------------
		void Video::exit()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Video::resizeBack(ofResizeEventArgs & args)
		{
			this->dirtyBounds = true;
		}

		//--------------------------------------------------------------
		void Video::resizeFront(ofResizeEventArgs & args)
		{
			this->dirtyBounds = true;
		}

		//--------------------------------------------------------------
		void Video::update(double dt)
		{
			this->layout = static_cast<render::Layout>(this->parameters.layout.get());
			
			if (this->dirtyBounds)
			{
				this->updateBounds();
			}

			this->videoPlayer.update();
			if (this->videoPlayer.isStopped() && this->parameters.playback.play)
			{
				this->parameters.playback.play.set(false);
			}
		}

		//--------------------------------------------------------------
		void Video::drawBackBase()
		{
			if (this->layout == render::Layout::Back)
			{
				this->drawVideo();
			}
		}

		//--------------------------------------------------------------
		void Video::drawFrontBase()
		{
			if (this->layout == render::Layout::Front)
			{
				this->drawVideo();
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

				ImGui::Text("Layout");
				static vector<string> layouts
				{ 
					"Back", 
					"Front" 
				};
				if (ofxPreset::Gui::AddRadio(this->parameters.layout, layouts, 2))
				{
					this->dirtyBounds = true;
				}

				static vector<string> contentModes
				{ 
					"Center", 
					"Top Left", 
					"Scale To Fill", 
					"Scale Aspect Fill", 
					"Scale Aspect Fit" 
				};
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

		//--------------------------------------------------------------
		void Video::drawVideo()
		{
			if (this->videoPlayer.isLoaded())
			{
				this->videoPlayer.draw(this->drawBounds.x, this->drawBounds.y, this->drawBounds.width, this->drawBounds.height);
			}
		}

		//--------------------------------------------------------------
		void Video::updateBounds()
		{
			const auto canvasWidth = GetCanvasWidth(layout);
			const auto canvasHeight = GetCanvasHeight(layout);

			const auto contentMode = static_cast<ContentMode>(this->parameters.contentMode.get());
			if (contentMode == ContentMode::Center)
			{
				this->drawBounds.setFromCenter(canvasWidth * 0.5f, canvasHeight * 0.5f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
			}
			else if (contentMode == ContentMode::TopLeft)
			{
				this->drawBounds.set(0.0f, 0.0f, this->videoPlayer.getWidth(), this->videoPlayer.getHeight());
			}
			else if (contentMode == ContentMode::ScaleToFill)
			{
				this->drawBounds.set(0.0f, 0.0f, canvasWidth, canvasHeight);
			}
			else
			{
				const auto canvasRatio = canvasWidth / canvasHeight;
				const auto videoRatio = this->videoPlayer.getWidth() / this->videoPlayer.getHeight();

				if (!(canvasRatio > videoRatio) ^ !(contentMode == ContentMode::ScaleAspectFit))
				{
					this->drawBounds.width = canvasWidth;
					this->drawBounds.height = this->drawBounds.width / videoRatio;
					this->drawBounds.x = 0.0f;
					this->drawBounds.y = (canvasHeight - this->drawBounds.height) * 0.5f;
				}
				else
				{
					this->drawBounds.height = canvasHeight;
					this->drawBounds.width = this->drawBounds.height * videoRatio;
					this->drawBounds.y = 0.0f;
					this->drawBounds.x = (canvasWidth - this->drawBounds.width) * 0.5f;
				}
			}

			this->dirtyBounds = false;
		}
	}
}