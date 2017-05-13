#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofVideoPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		class Video 
			: public Base
		{
		public:
			Video();
			virtual ~Video();

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;
			
			void gui(ofxImGui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadVideo(const std::filesystem::path & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			ofVideoPlayer videoPlayer;

			string fileName;
			bool wasLoaded;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> filePath{ "File Path", "" };
				ofParameter<bool> loop{ "Loop", false };
				ofParameter<bool> scrubToTimeline{ "Scrub To Timeline", true };

				PARAM_DECLARE("Video", 
					filePath, 
					loop,
					scrubToTimeline);
			} parameters;
		};
	}
}
