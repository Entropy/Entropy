#include "Video.h"

#include "ofGstVideoPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Video::Video()
			: Base(Type::Video)
			, wasLoaded(false)
		{
			this->videoPlayer.setPlayer(std::shared_ptr<ofGstVideoPlayer>(new ofGstVideoPlayer()));
		}

		//--------------------------------------------------------------
		Video::~Video()
		{}

		//--------------------------------------------------------------
		void Video::init()
		{
			this->parameterListeners.push_back(this->parameters.loop.newListener([this](bool & enabled)
			{
				this->videoPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}));
		}

		//--------------------------------------------------------------
		void Video::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Video::exit()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Video::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}
			
			this->videoPlayer.update();

			if (!this->isLoaded())
			{
				return;
			}

			if (this->switchMillis >= 0.0f)
			{
				float durationMillis = this->videoPlayer.getDuration() * 1000.0f;
				bool shouldPlay = durationMillis > 0.0 && (this->parameters.loop || durationMillis >= this->switchMillis);
				
				if (this->timeline->getIsPlaying())
				{
					if (shouldPlay && this->videoPlayer.isPaused())
					{
						// Set the starting position.
						float positionMillis = this->switchMillis;
						while (positionMillis > durationMillis)
						{
							positionMillis -= durationMillis;
						}

						this->videoPlayer.setPosition(positionMillis / durationMillis);
						this->videoPlayer.setPaused(false);
					}
					else if (!shouldPlay && !this->videoPlayer.isPaused())
					{
						this->videoPlayer.setPaused(true);
					}
				}
				else if (shouldPlay && this->parameters.scrubToTimeline)
				{
					// Scrub the video.
					float positionMillis = this->switchMillis;
					while (positionMillis > durationMillis)
					{
						positionMillis -= durationMillis;
					}

					this->videoPlayer.setPosition(positionMillis / durationMillis);
					this->videoPlayer.setPaused(false);
				}
			}
			else if (this->switchMillis < 0.0f && !this->videoPlayer.isPaused())
			{
				this->videoPlayer.setPaused(true);
			}
		}

		//--------------------------------------------------------------
		void Video::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ofxPreset::Gui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a video file.", false, GetSharedAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadVideo(result.filePath))
						{
							auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
							auto testPath = GetSharedAssetsPath().append(relativePath);
							if (ofFile::doesFileExist(testPath))
							{
								this->parameters.filePath = relativePath;
							}
							else
							{
								this->parameters.filePath = result.filePath;
							}
						}
					}
				}
				ImGui::Text("Filename: %s", this->fileName.c_str());

				ofxPreset::Gui::EndTree(settings);
			}

			ofxPreset::Gui::AddParameter(this->parameters.loop);
			ofxPreset::Gui::AddParameter(this->parameters.scrubToTimeline);
		}

		//--------------------------------------------------------------
		void Video::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadVideo(filePath);
				}
				else
				{
					this->loadVideo(GetSharedAssetsPath() + filePath);
				}
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
			
			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->videoPlayer.loadAsync(filePath);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			this->videoPlayer.play();
			this->videoPlayer.setLoopState(this->parameters.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Video::isLoaded() const
		{
			return (this->videoPlayer.isLoaded() && this->getContentWidth() > 0.0f && this->getContentHeight() > 0.0f);
		}

		//--------------------------------------------------------------
		float Video::getContentWidth() const
		{
			return this->videoPlayer.getWidth();
		}

		//--------------------------------------------------------------
		float Video::getContentHeight() const
		{
			return this->videoPlayer.getHeight();
		}

		//--------------------------------------------------------------
		void Video::renderContent()
		{
			if (this->isLoaded() && !this->videoPlayer.isPaused())
			{
				this->videoPlayer.getTexture().drawSubsection(this->dstBounds, this->srcBounds);
			}
		}
	}
}
