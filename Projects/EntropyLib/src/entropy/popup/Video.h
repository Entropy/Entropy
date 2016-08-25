#pragma once

#include "Base.h"

#include "ofxPreset.h"
#ifdef TARGET_WIN32
#include "ofxWMFVideoPlayer.h"
#endif

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

			void setup() override;
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

#ifdef TARGET_WIN32
			ofxWMFVideoPlayer video;
#else
			ofVideoPlayer video;
#endif
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

				PARAM_DECLARE("Video", filePath);
			} parameters;
		};
	}
}
