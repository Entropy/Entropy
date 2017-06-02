#pragma once

#include "ofFbo.h"
#include "ofParameter.h"
#include "ofShader.h"
#include "entropy/Helpers.h"

namespace entropy
{
	namespace post
	{
		struct ColorParameters
			: ofParameterGroup
		{
			ofParameter<float> contrast{ "Contrast", 1.0f, 0.5f, 1.5f };
			ofParameter<float> brightness{ "Brightness", 0.0f, -1.0f, 1.0f };

			PARAM_DECLARE("Color",
				contrast,
				brightness);
		};

		class ColorEffects
		{
		public:
			ColorEffects();
			~ColorEffects();

			void begin();
			void end();

			void setParams(const ColorParameters & parameters);

			std::filesystem::path getShaderPath(const string & shaderFile = "");

		protected:
			ofShader::Settings colorSettings;
			ofShader colorShader;
			std::time_t colorTime;
		};
	}
}