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
			
			void gui(ofxImGui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadHPV(const std::filesystem::path & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			ofxHPVPlayer hpvPlayer;

			string fileName;
			bool wasLoaded;
			bool renderFrame;

			uint64_t freePlayStartMillis;
			int freePlayStartFrame;
			bool freePlayInit;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> filePath{ "File Path", "" };
				ofParameter<bool> loop{ "Loop", false };
				ofParameter<bool> syncToTimeline{ "Sync To Timeline", true };

				PARAM_DECLARE("HPV", 
					filePath, 
					loop,
					syncToTimeline);
			} parameters;
		};
	}
}
