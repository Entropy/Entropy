#include "Movie.h"

#include "ofGstVideoPlayer.h"

#include "entropy/Helpers.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Movie::Movie()
			: Base(Type::Movie)
			, wasLoaded(false)
		{
			this->videoPlayer.setPlayer(std::make_shared<ofGstVideoPlayer>());
		}

		//--------------------------------------------------------------
		Movie::~Movie()
		{}

		//--------------------------------------------------------------
		void Movie::init()
		{
			this->parameterListeners.push_back(this->parameters.loop.newListener([this](bool & enabled)
			{
				this->videoPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}));
		}

		//--------------------------------------------------------------
		void Movie::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Movie::exit()
		{
			this->videoPlayer.close();
		}

		//--------------------------------------------------------------
		void Movie::update(double dt)
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
		void Movie::gui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a movie file.", false, GetSharedAssetsPath().string());
					if (result.bSuccess)
					{
						if (this->loadVideo(result.filePath))
						{
							const auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
							const auto testPath = GetSharedAssetsPath().append(relativePath);
							if (ofFile::doesFileExist(testPath.string()))
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

				ofxImGui::EndTree(settings);
			}

			ofxImGui::AddParameter(this->parameters.loop);
			ofxImGui::AddParameter(this->parameters.scrubToTimeline);
		}

		//--------------------------------------------------------------
		void Movie::deserialize(const nlohmann::json & json)
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
					this->loadVideo(GetSharedAssetsPath().string() + filePath);
				}
			}
		}

		//--------------------------------------------------------------
		bool Movie::loadVideo(const std::filesystem::path & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No file found at " << filePath;
				return false;
			}
			
			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->videoPlayer.loadAsync(filePath.string());
			}
			if (wasUsingArbTex) ofEnableArbTex();

			this->videoPlayer.play();
			this->videoPlayer.setLoopState(this->parameters.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Movie::isLoaded() const
		{
			return (this->videoPlayer.isLoaded() && this->getContentWidth() > 0.0f && this->getContentHeight() > 0.0f);
		}

		//--------------------------------------------------------------
		float Movie::getContentWidth() const
		{
			return this->videoPlayer.getWidth();
		}

		//--------------------------------------------------------------
		float Movie::getContentHeight() const
		{
			return this->videoPlayer.getHeight();
		}

		//--------------------------------------------------------------
		void Movie::renderContent()
		{
			if (this->isLoaded() && !this->videoPlayer.isPaused())
			{
				this->videoPlayer.getTexture().drawSubsection(this->dstBounds, this->srcBounds);
			}
		}

		//--------------------------------------------------------------
		unsigned long long Movie::getContentDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->videoPlayer.getDuration() * 1000;
			}
			return 0;
		}
	}
}
