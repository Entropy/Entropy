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

			void exit() override;
			
			void gui(ofxImGui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadImage(const string & filePath);

			bool isLoaded() const override;

			float getContentWidth() const override;
			float getContentHeight() const override;
			void renderContent() override;

			ofTexture image;
			string fileName;

		protected:
			BaseParameters & getParameters() override
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