#pragma once

#include "Base.h"

#include "ofFileUtils.h"
#include "ofxImageSequence.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace media
	{
		class Sequence
			: public Base
		{
		public:
			Sequence();
			virtual ~Sequence();

			void init() override;

			void setup() override;
			void exit() override;

			void update(double dt) override;

			void gui(ofxImGui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadSequence(const std::filesystem::path & folderPath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			unsigned long long getContentDurationMs() const override;

			ofxImageSequence imageSequence;

			string folderName;
			bool wasLoaded;
			bool playing;
			float startTimeMillis;
			float startPosMillis;
			bool renderFrame;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> folderPath{ "Folder Path", "" };
				ofParameter<int> fps{ "FPS", 30, 1, 120 };
				ofParameter<bool> loop{ "Loop", false };
				ofParameter<bool> syncToTimeline{ "Sync To Timeline", true };

				PARAM_DECLARE("Sequence",
					folderPath,
					fps,
					loop,
					syncToTimeline);
			} parameters;
		};
	}
}
