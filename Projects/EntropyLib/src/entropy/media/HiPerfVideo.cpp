#include "HiPerfVideo.h"

#include "entropy/Helpers.h"
#include "ofxTimeline.h"
#include "App.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		HiPerfVideo::HiPerfVideo()
			: Base(Type::HPV)
			, wasLoaded(false)
		{
			// Engine initialized in ofApp::setup().
			//HPV::InitHPVEngine();
			this->hpvPlayer.init(HPV::NewPlayer());
		}

		//--------------------------------------------------------------
		HiPerfVideo::~HiPerfVideo()
		{}

		//--------------------------------------------------------------
		void HiPerfVideo::init()
		{
			this->parameterListeners.push_back(this->parameters.loop.newListener([this](bool & enabled)
			{
				this->hpvPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}));
		}

		//--------------------------------------------------------------
		void HiPerfVideo::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void HiPerfVideo::exit()
		{
			this->hpvPlayer.close();
		}

		//--------------------------------------------------------------
		void HiPerfVideo::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}
			
			if (!this->isLoaded())
			{
				return;
			}

			if (this->switchMillis >= 0.0f)
			{
				this->renderFrame = true;
				
				float durationMillis = this->hpvPlayer.getDuration() * 1000.0f;
				bool shouldPlay = durationMillis > 0.0f && (this->parameters.loop || durationMillis >= this->switchMillis);
				
				if (this->parameters.syncToTimeline)
				{
					if (!this->hpvPlayer.isPaused())
					{
						this->hpvPlayer.setPaused(true);
					}

					if (shouldPlay)
					{
						// Scrub the video.
						float positionMillis = this->switchMillis;
						while (positionMillis > durationMillis)
						{
							positionMillis -= durationMillis;
						}
						this->hpvPlayer.setPosition(positionMillis / durationMillis);
					}
				}
				else
				{
					if (shouldPlay && this->hpvPlayer.isPaused())
					{
						// Set the starting position.
						float positionMillis = this->switchMillis;
						while (positionMillis > durationMillis)
						{
							positionMillis -= durationMillis;
						}

						this->hpvPlayer.setPosition(positionMillis / durationMillis);
						this->hpvPlayer.setPaused(false);
					}
					else if (!shouldPlay && !this->hpvPlayer.isPaused())
					{
						this->hpvPlayer.setPaused(true);
					}
				}
			}
			else if (!this->hpvPlayer.isPaused())
			{
				this->renderFrame = false;

				this->hpvPlayer.setPaused(true);
			}
		}

		//--------------------------------------------------------------
		void HiPerfVideo::gui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select an hpv file.", false, GetSharedAssetsPath().string());
					if (result.bSuccess)
					{
						if (this->loadHPV(result.filePath))
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
			ofxImGui::AddParameter(this->parameters.syncToTimeline);
		}

		//--------------------------------------------------------------
		void HiPerfVideo::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadHPV(filePath);
				}
				else
				{
					this->loadHPV(GetSharedAssetsPath().string() + filePath);
				}
			}
		}

		//--------------------------------------------------------------
		bool HiPerfVideo::loadHPV(const std::filesystem::path & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No file found at " << filePath;
				return false;
			}
			
			this->hpvPlayer.close();
			this->hpvPlayer.loadAsync(filePath.string());
			
			this->hpvPlayer.play();
			this->hpvPlayer.setPaused(false);
			this->hpvPlayer.setLoopState(this->parameters.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool HiPerfVideo::isLoaded() const
		{
			return (this->hpvPlayer.isLoaded() && this->getContentWidth() > 0.0f && this->getContentHeight() > 0.0f);
		}

		//--------------------------------------------------------------
		float HiPerfVideo::getContentWidth() const
		{
			return this->hpvPlayer.getWidth();
		}

		//--------------------------------------------------------------
		float HiPerfVideo::getContentHeight() const
		{
			return this->hpvPlayer.getHeight();
		}

		//--------------------------------------------------------------
		void HiPerfVideo::renderContent()
		{
			if (this->isLoaded() && this->renderFrame)
			{
				this->hpvPlayer.drawSubsection(this->dstBounds, this->srcBounds);
			}
		}
	}
}
