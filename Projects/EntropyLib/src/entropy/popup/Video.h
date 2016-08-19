#pragma once

#include "Base.h"

#include "ofxPreset.h"
#include "ofxWMFVideoPlayer.h"

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

			void exit() override;

			void update(double dt) override;
			
			void gui(ofxPreset::Gui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadVideo(const string & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			ofxWMFVideoPlayer video;
			string fileName;

		protected:
			BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> filePath{ "File Path", "" };

				PARAM_DECLARE("Video", filePath);
			} parameters;
		};
	}
}