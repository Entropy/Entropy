#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofSoundPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace media
	{
		class Sound
			: public Base
		{
		public:
			Sound();
			virtual ~Sound();

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			uint64_t getCurrentTimeMs() const override;
			uint64_t getCurrentFrame() const override;

			uint64_t getPlaybackFrame() override;

			uint64_t getDurationMs() const override;
			uint64_t getDurationFrames() const override;

			uint64_t getFrameRate() const override;

		protected:
			bool loadMedia(const std::filesystem::path & filePath) override;
			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			bool initFreePlay() override;

			ofSoundPlayer soundPlayer;
		};
	}
}