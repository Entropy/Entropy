#include "ColorEffects.h"

#include "ofGraphics.h"

namespace entropy
{
	namespace post
	{
		//--------------------------------------------------------------
		ColorEffects::ColorEffects()
		{
			// Load shaders.
			this->colorSettings.shaderFiles[GL_VERTEX_SHADER] = this->getShaderPath("vert_passthru.glsl");
			this->colorSettings.shaderFiles[GL_FRAGMENT_SHADER] = this->getShaderPath("frag_color.glsl");
			this->colorShader.setup(colorSettings);
		}

		//--------------------------------------------------------------
		ColorEffects::~ColorEffects()
		{}

		//--------------------------------------------------------------
		void ColorEffects::begin()
		{			
			auto vertTime = std::filesystem::last_write_time(this->colorSettings.shaderFiles[GL_VERTEX_SHADER]);
			auto fragTime = std::filesystem::last_write_time(this->colorSettings.shaderFiles[GL_FRAGMENT_SHADER]);
			if (vertTime > this->colorTime || fragTime > this->colorTime)
			{
				this->colorTime = std::max(vertTime, fragTime);
				this->colorShader.setup(colorSettings);
			}
			
			this->colorShader.begin();
			this->colorShader.bindDefaults();
		}

		//--------------------------------------------------------------
		void ColorEffects::end()
		{
			this->colorShader.end();
		}

		//--------------------------------------------------------------
		void ColorEffects::setParams(const ColorParameters & parameters)
		{
			this->colorShader.setUniform1f("brightness", parameters.brightness);
			this->colorShader.setUniform1f("contrast", parameters.contrast);
		}

		//--------------------------------------------------------------
		std::filesystem::path ColorEffects::getShaderPath(const string & shaderFile)
		{
			return GetShadersPath(Module::PostEffects) / shaderFile;
		}
	}
}