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
			: Asset(Type::Movie)
		{
			this->videoPlayer.setPlayer(std::make_shared<ofGstVideoPlayer>());
		}

		//--------------------------------------------------------------
		Movie::~Movie()
		{}

		//--------------------------------------------------------------
		void Movie::init()
		{
			this->parameterListeners.push_back(this->parameters.playback.loop.newListener([this](bool & enabled)
			{
				this->videoPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}));
			this->parameterListeners.push_back(this->parameters.playback.syncMode.newListener([this](int & mode)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::Timeline)
				{
					this->parameters.playback.freeSpeed = 1.0f;
					this->videoPlayer.setSpeed(1.0f);
				}
				else if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
				{
					this->videoPlayer.setSpeed(this->parameters.playback.freeSpeed);
				}
				else if (this->getLinkedMedia() != nullptr)
				{
					this->videoPlayer.setSpeed(this->getLinkedMedia()->parameters.playback.freeSpeed);
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.freeSpeed.newListener([this](float & speed)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
				{
					this->videoPlayer.setSpeed(speed);
				}
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
				// Add a new switch if none exist.
				this->addDefaultSwitch();
				
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}
			
			this->videoPlayer.update();

			if (!this->isLoaded()) return;

			if (this->switchMillis >= 0.0f)
			{
				this->renderFrame = true;
			}

			const bool shouldPlay = this->shouldPlay();
			if (shouldPlay)
			{
				if (this->videoPlayer.isPaused())
				{
					this->videoPlayer.setFrame(this->getPlaybackFrame());
					this->videoPlayer.setPaused(false);
				}

				const auto syncMode = this->getSyncMode();
				if (syncMode != SyncMode::FreePlay && syncMode != SyncMode::FadeControl)
				{
					this->videoPlayer.setFrame(this->getPlaybackFrame());
				}
			}
			else if (!this->videoPlayer.isPaused())
			{
				this->videoPlayer.setPaused(true);
				this->freePlayNeedsInit = true;
			}
		}

		//--------------------------------------------------------------
		bool Movie::loadMedia(const std::filesystem::path & filePath)
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
			this->videoPlayer.setLoopState(this->parameters.playback.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

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
			if (this->isLoaded() && this->renderFrame)
			{
				this->videoPlayer.getTexture().drawSubsection(this->dstBounds, this->srcBounds);
			}
		}

		//--------------------------------------------------------------
		bool Movie::initFreePlay()
		{
			if (this->freePlayNeedsInit)
			{
				// Get start time and frames for free play.
				this->freePlayElapsedLastMs = ofGetElapsedTimeMillis();

				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay)
				{
					const uint64_t durationMs = this->getDurationMs();
					this->freePlayMediaStartMs = std::max(0.0f, this->switchMillis);
					while (this->freePlayMediaStartMs > durationMs)
					{
						this->freePlayMediaStartMs -= durationMs;
					}

					this->freePlayMediaStartFrame = (this->freePlayMediaStartMs / static_cast<float>(durationMs)) * this->videoPlayer.getTotalNumFrames();
				}
				else if (syncMode == SyncMode::FadeControl)
				{
					this->freePlayMediaStartMs = 0;
					this->freePlayMediaStartFrame = 0;
				}

				this->freePlayNeedsInit = false;

				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getCurrentTimeMs() const
		{
			if (this->isLoaded())
			{
				return this->videoPlayer.getPosition() * this->videoPlayer.getDuration() * 1000.0f;
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getCurrentFrame() const
		{
			if (this->isLoaded())
			{
				return this->videoPlayer.getCurrentFrame();
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->videoPlayer.getDuration() * 1000;
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getDurationFrames() const
		{
			if (this->isLoaded())
			{
				return this->videoPlayer.getTotalNumFrames();
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getFrameRate() const
		{
			return this->videoPlayer.getTotalNumFrames() / this->videoPlayer.getDuration();
		}
	}
}
