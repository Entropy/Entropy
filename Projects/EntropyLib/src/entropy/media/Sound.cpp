#include "Sound.h"

#include "ofGstVideoPlayer.h"

#include "entropy/Helpers.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Sound::Sound()
			: Base(Type::Sound)
		{}

		//--------------------------------------------------------------
		Sound::~Sound()
		{}

		//--------------------------------------------------------------
		void Sound::init()
		{
			this->parameterListeners.push_back(this->parameters.playback.fadeTrack.newListener([this](float &)
			{
				this->soundPlayer.setVolume(this->getTotalFade());
			}));
			this->parameterListeners.push_back(this->parameters.playback.fadeTwist.newListener([this](float &)
			{
				this->soundPlayer.setVolume(this->getTotalFade());
			}));
			this->parameterListeners.push_back(this->parameters.playback.loop.newListener([this](bool & enabled)
			{
				this->soundPlayer.setLoop(enabled);
			}));
			this->parameterListeners.push_back(this->parameters.playback.syncMode.newListener([this](int & mode)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::Timeline)
				{
					this->parameters.playback.freeSpeed = 1.0f;
					this->soundPlayer.setSpeed(1.0f);
				}
				else if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
				{
					this->soundPlayer.setSpeed(this->parameters.playback.freeSpeed);
				}
				else if (this->getLinkedMedia() != nullptr)
				{
					this->soundPlayer.setSpeed(this->getLinkedMedia()->parameters.playback.freeSpeed);
				}
			}));
			this->parameterListeners.push_back(this->parameters.playback.freeSpeed.newListener([this](float & speed)
			{
				const auto syncMode = this->getSyncMode();
				if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
				{
					this->soundPlayer.setSpeed(speed);
				}
			}));
		}

		//--------------------------------------------------------------
		void Sound::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Sound::exit()
		{
			this->soundPlayer.unload();
		}

		//--------------------------------------------------------------
		void Sound::update(double dt)
		{
			if (!this->isLoaded()) return;
			
			if (!this->wasLoaded)
			{
				// Add a new switch if none exist.
				this->addDefaultSwitch(); 
				this->wasLoaded = true;
			}
			
			// Sound is special because it's the master.
			const bool shouldPlay = this->shouldPlay() && this->timeline->getIsPlaying();
			if (shouldPlay)
			{
				const auto syncMode = this->getSyncMode();

				if (!this->soundPlayer.isPlaying())
				{
					this->soundPlayer.play();
					if (syncMode == SyncMode::FreePlay || syncMode == SyncMode::FadeControl)
					{
						this->soundPlayer.setPositionMS(this->getPlaybackTimeMs());
					}
				}

				if (syncMode != SyncMode::FreePlay && syncMode != SyncMode::FadeControl)
				{
					this->soundPlayer.setPositionMS(this->getPlaybackTimeMs());
				}
			}
			else if (this->soundPlayer.isPlaying())
			{
				this->soundPlayer.stop();
				this->freePlayNeedsInit = true;
			}
		}

		//--------------------------------------------------------------
		bool Sound::loadMedia(const std::filesystem::path & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No audio found at " << filePath;
				return false;
			}

			this->soundPlayer.load(filePath.string());
			this->soundPlayer.setLoop(this->parameters.playback.loop);

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Sound::isLoaded() const
		{
			return (this->soundPlayer.isLoaded());
		}

		//--------------------------------------------------------------
		float Sound::getContentWidth() const
		{
			return 0.0f;
		}

		//--------------------------------------------------------------
		float Sound::getContentHeight() const
		{
			return 0.0f;
		}

		//--------------------------------------------------------------
		void Sound::renderContent()
		{}

		//--------------------------------------------------------------
		bool Sound::initFreePlay()
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
				}
				else if (syncMode == SyncMode::FadeControl)
				{
					this->freePlayMediaStartMs = 0;
				}

				this->freePlayMediaStartFrame = 0;
				
				this->freePlayNeedsInit = false;

				return true;
			}
			return false;
		}

		//--------------------------------------------------------------
		uint64_t Sound::getCurrentTimeMs() const
		{
			if (this->isLoaded())
			{
				return this->soundPlayer.getPositionMS();
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Sound::getCurrentFrame() const
		{
			return 0;
		}
		
		//--------------------------------------------------------------
		uint64_t Sound::getPlaybackFrame()
		{
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Sound::getDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->soundPlayer.getDurationMS();
			}
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Sound::getDurationFrames() const
		{
			return 1;
		}

		//--------------------------------------------------------------
		uint64_t Sound::getFrameRate() const
		{
			return 1;
		}
	}
}
