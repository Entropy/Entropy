#pragma once

#include "Base.h"

#include "ofxPreset.h"

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

			void setup();
			void exit();

			void update(double dt);
			
			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			bool loadVideo(const string & filePath);

			ofTexture & getTexture() override;

			ofVideoPlayer video;
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