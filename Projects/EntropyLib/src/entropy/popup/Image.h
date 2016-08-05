#pragma once

#include "Base.h"

#include "ofImage.h"
#include "ofxPreset.h"

namespace entropy
{
	namespace popup
	{
		class Image 
			: public Base
		{
		public:
			Image();
			virtual ~Image();

			void setup();
			void exit();
			
			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			bool loadImage(const string & filePath);

			virtual ofTexture & getTexture() override;

			ofTexture image;
			string fileName;

		protected:
			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> filePath{ "File Path", "" };

				PARAM_DECLARE("Image", filePath);
			} parameters;
		};
	}
}