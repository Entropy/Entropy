#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofVideoPlayer.h"

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

			void gui(ofxImGui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadSound(const std::filesystem::path & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			unsigned long long getContentDurationMs() const override;

			ofVideoPlayer soundPlayer;

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

				PARAM_DECLARE("Sound",
					filePath,
					loop);
			} parameters;
		};
	}
}