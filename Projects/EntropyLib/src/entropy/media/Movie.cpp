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
			if (this->isLoaded() && !this->videoPlayer.isPaused())
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
				this->freePlayStartElapsedMs = ofGetElapsedTimeMillis();

				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay)
				{
					const uint64_t durationMs = this->getDurationMs();
					this->freePlayStartMediaMs = std::max(0.0f, this->switchMillis);
					while (this->freePlayStartMediaMs > durationMs)
					{
						this->freePlayStartMediaMs -= durationMs;
					}

					this->freePlayStartMediaFrame = (this->freePlayStartMediaMs / static_cast<float>(durationMs)) * this->videoPlayer.getTotalNumFrames();
				}
				else if (syncMode == SyncMode::FadeControl)
				{
					this->freePlayStartMediaMs = 0;
					this->freePlayStartMediaFrame = 0;
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
		uint64_t Movie::getPlaybackTimeMs()
		{
			const auto syncMode = this->getSyncMode();

			if (syncMode == SyncMode::Timeline)
			{
				const uint64_t durationMs = this->getDurationMs();
				if (durationMs == 0) return 0;

				uint64_t positionMs = this->switchMillis;
				while (positionMs > durationMs)
				{
					positionMs -= durationMs;
				}
				return positionMs;
			}

			if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
			{
				if (this->initFreePlay())
				{
					return this->freePlayStartMediaMs;
				}

				return (ofGetElapsedTimeMillis() - this->freePlayStartElapsedMs + this->freePlayStartMediaMs);
			}

			//else SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackTimeMs();
			}

			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Movie::getPlaybackFrame()
		{
			const auto syncMode = this->getSyncMode();

			if (syncMode == SyncMode::Timeline)
			{
				return (this->getPlaybackTimeMs() / static_cast<float>(this->getDurationMs())) * this->getDurationFrames();
			}

			if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
			{
				if (this->initFreePlay())
				{
					return this->freePlayStartMediaFrame;
				}

				return (this->getPlaybackTimeMs() / static_cast<float>(this->getDurationMs())) * this->getDurationFrames();
			}

			//else SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackFrame();
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
	}
}
