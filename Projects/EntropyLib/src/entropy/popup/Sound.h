#pragma once

#include "Base.h"

#include "ofxPreset.h"

namespace entropy
{
	namespace popup
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

			void gui(ofxPreset::Gui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadSound(const string & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

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