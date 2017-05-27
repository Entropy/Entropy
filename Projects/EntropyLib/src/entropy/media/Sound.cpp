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
			this->parameterListeners.push_back(this->parameters.playback.fade.newListener([this](float & val)
			{
				this->soundPlayer.setVolume(val);
			}));
			this->parameterListeners.push_back(this->parameters.playback.loop.newListener([this](bool & enabled)
			{
				this->soundPlayer.setLoop(enabled);
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
			
			const bool shouldPlay = this->shouldPlay();
			if (shouldPlay)
			{
				if (!this->soundPlayer.isPlaying())
				{
					this->soundPlayer.play();
				}

				const auto syncMode = this->getSyncMode();
				if (syncMode != SyncMode::FreePlay)
				{
					this->soundPlayer.setPositionMS(this->getPlaybackTimeMs());
				}
			}
			else if (this->soundPlayer.isPlaying())
			{
				this->soundPlayer.stop();
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
				this->freePlayStartElapsedMs = ofGetElapsedTimeMillis();

				const uint64_t durationMs = this->getDurationMs();
				this->freePlayStartMediaMs = std::max(0.0f, this->switchMillis);
				while (this->freePlayStartMediaMs > durationMs)
				{
					this->freePlayStartMediaMs -= durationMs;
				}

				this->freePlayStartMediaFrame = 0;
				
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
		uint64_t Sound::getPlaybackTimeMs()
		{
			const auto syncMode = this->getSyncMode();

			if (syncMode == SyncMode::Timeline)
			{
				const uint64_t durationMs = this->getDurationMs();
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
			return 0;
		}
	}
}
