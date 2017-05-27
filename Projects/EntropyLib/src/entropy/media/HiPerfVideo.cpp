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
			this->parameterListeners.push_back(this->parameters.playback.loop.newListener([this](bool & enabled)
			{
				if (this->hpvPlayer.isLoaded())
				{
					this->hpvPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
				}
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
			if (!this->isLoaded()) return;
			
			if (!wasLoaded)
			{
				// Add a new switch if none exist.
				this->addDefaultSwitch();
				
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}
			
			const bool shouldPlay = this->shouldPlay();
			if (shouldPlay)
			{
				this->renderFrame = true;

				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::Timeline)
				{
					this->hpvPlayer.setPosition(this->getPlaybackTimeMs() / this->getDurationMs());
				}
				else
				{
					this->hpvPlayer.setFrame(this->getPlaybackFrame());
				}
			}
			else
			{
				this->renderFrame = false;
			}
		}

		//--------------------------------------------------------------
		bool HiPerfVideo::loadMedia(const std::filesystem::path & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No file found at " << filePath;
				return false;
			}
			
			this->hpvPlayer.close();
			this->hpvPlayer.init(HPV::NewPlayer());
			this->hpvPlayer.loadAsync(filePath.string());
			
			this->hpvPlayer.play();
			this->hpvPlayer.setPaused(false);
			this->hpvPlayer.setLoopState(this->parameters.playback.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

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

		//--------------------------------------------------------------
		bool HiPerfVideo::initFreePlay()
		{
			if (this->freePlayNeedsInit)
			{
				// Get start time and frames for free play.
				this->freePlayStartElapsedMs = ofGetElapsedTimeMillis();

				const uint64_t durationMs = this->getDurationMs();
				this->freePlayStartMediaMs = std::max(0.0f, this->switchMillis);
				while (this->freePlayStartMediaMs > durationMs)
				{
					this->freePlayStartMediaMs -= durationMs;
				}

				this->freePlayStartMediaFrame = (this->freePlayStartMediaMs / 1000.0f) * this->hpvPlayer.getFrameRate();

				this->freePlayNeedsInit = false;

				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getCurrentTimeMs() const
		{
			if (this->isLoaded())
			{
				return this->hpvPlayer.getPosition() * this->hpvPlayer.getDuration() * 1000.0f;
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getCurrentFrame() const
		{
			if (this->isLoaded())
			{
				return this->hpvPlayer.getCurrentFrame();
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getPlaybackTimeMs()
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

			if (syncMode == SyncMode::FreePlay)
			{
				if (this->initFreePlay())
				{
					return this->freePlayStartMediaMs;
				}

				return ofGetElapsedTimeMillis() - this->freePlayStartMediaMs;
			}

			//else SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackTimeMs();
			}

			return 0;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getPlaybackFrame()
		{
			const auto syncMode = this->getSyncMode();

			if (syncMode == SyncMode::Timeline)
			{
				return (this->getPlaybackTimeMs() / static_cast<float>(this->getDurationMs())) * this->getDurationFrames();
			}

			if (syncMode == SyncMode::FreePlay)
			{
				if (this->initFreePlay())
				{
					return this->freePlayStartMediaFrame;
				}


				return (this->getPlaybackTimeMs() / 1000.0f * this->hpvPlayer.getFrameRate());
			}

			//else SyncMode::LinkedMedia
			if (this->linkedMedia != nullptr)
			{
				return this->linkedMedia->getPlaybackFrame();
			}

			return 0;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->hpvPlayer.getDuration() * 1000;
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t HiPerfVideo::getDurationFrames() const
		{
			if (this->isLoaded())
			{
				return this->hpvPlayer.getTotalNumFrames();
			}
			return 0;
		}
	}
}
