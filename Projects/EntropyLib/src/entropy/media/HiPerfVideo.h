#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofxHPVPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace media
	{
		class HiPerfVideo
			: public Base
		{
		public:
			HiPerfVideo();
			virtual ~HiPerfVideo();

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			uint64_t getCurrentTimeMs() const override;
			uint64_t getCurrentFrame() const override;

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

			ofxHPVPlayer hpvPlayer;

			bool renderFrame;
		};
	}
}
