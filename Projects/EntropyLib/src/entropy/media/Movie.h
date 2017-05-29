#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofVideoPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace media
	{
		class Movie 
			: public Base
		{
		public:
			Movie();
			virtual ~Movie();

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			uint64_t getCurrentTimeMs() const override;
			uint64_t getCurrentFrame() const override;

			uint64_t getPlaybackTimeMs() override;
			uint64_t getPlaybackFrame() override;

			uint64_t getDurationMs() const override;
			uint64_t getDurationFrames() const override;

		protected:
			bool loadMedia(const std::filesystem::path & filePath) override;
			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			bool initFreePlay() override;

			ofVideoPlayer videoPlayer;

			bool renderFrame;
		};
	}
}
