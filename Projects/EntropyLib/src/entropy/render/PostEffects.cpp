#include "PostEffects.h"

#include "entropy/Helpers.h"

//--------------------------------------------------------------
float gaussian(float x, float mu, float sigma) 
{
	auto d = x - mu;
	auto n = 1.0 / (sqrtf(glm::two_pi<float>()) * sigma);
	return exp(-d * d / (2.0 * sigma * sigma)) * n;
}

namespace entropy
{
	namespace render
	{

		//--------------------------------------------------------------
		PostEffects::PostEffects()
		{
			// Load shaders.
			this->brightnessThresholdShader.load(this->getShaderPath("passthrough_vert.glsl"), this->getShaderPath("brightnessThreshold.frag"));

			auto blurVertFile = ofFile(this->getShaderPath("directionalBlur.vert"));
			auto blurFragFile = ofFile(this->getShaderPath("directionalBlur.frag"));
			auto blurVertSource = ofBuffer(blurVertFile);
			auto blurFragSource = ofBuffer(blurFragFile);

			constexpr auto blurHorzPrefix = "#version 330\n#define PASS_H\n#define BLUR9\n";
			this->blurHorzShader.setupShaderFromSource(GL_VERTEX_SHADER, blurHorzPrefix + blurVertSource.getText());
			this->blurHorzShader.setupShaderFromSource(GL_FRAGMENT_SHADER, blurHorzPrefix + blurFragSource.getText());
			this->blurHorzShader.bindDefaults();
			this->blurHorzShader.linkProgram();

			constexpr auto blurVertPrefix = "#version 330\n#define PASS_V\n#define BLUR9\n";
			this->blurVertShader.setupShaderFromSource(GL_VERTEX_SHADER, blurVertPrefix + blurVertSource.getText());
			this->blurVertShader.setupShaderFromSource(GL_FRAGMENT_SHADER, blurVertPrefix + blurFragSource.getText());
			this->blurVertShader.bindDefaults();
			this->blurVertShader.linkProgram();

            colorCorrectShaderSettings.shaderFiles[GL_VERTEX_SHADER] = this->getShaderPath("fullscreenTriangle.vert");
            colorCorrectShaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = this->getShaderPath("frag_tonemap.glsl");
			colorCorrectShaderSettings.intDefines["ENABLE_VIGNETTE"] = true;
			colorCorrectShaderSettings.intDefines["DEBUG_VIGNETTE"] = false;
			colorCorrectShaderSettings.intDefines["ONLY_ALPHA"] = false;
            colorCorrectShader.setup(colorCorrectShaderSettings);
		
			// Build render geometry.
			this->fullQuad.addVertices({ { -1, -1, 0 },{ -1,1,0 },{ 1,1,0 },{ 1,-1,0 } });
			this->fullQuad.addTexCoords({ { 0,1 },{ 0,0 },{ 1,0 },{ 1,1 } });
			this->fullQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

			glGenVertexArrays(1, &this->defaultVao);
		}
		
		//--------------------------------------------------------------
		PostEffects::~PostEffects()
		{
			for (int i = 0; i <  2; ++i)
			{
				this->fboTemp->clear();
			}

			this->fullQuad.clear();

			glDeleteVertexArrays(1, &this->defaultVao);
		}

		//--------------------------------------------------------------
        void PostEffects::process(const ofTexture & srcTexture, ofFbo & dstFbo, const entropy::render::PostParameters & parameters)
		{
            ofSetColor(255);
			if(parameters.vignette.enabled != colorCorrectShaderSettings.intDefines["ENABLE_VIGNETTE"]){
				colorCorrectShaderSettings.intDefines["ENABLE_VIGNETTE"] = parameters.vignette.enabled;
                colorCorrectShader.setup(colorCorrectShaderSettings);
            }

			if(parameters.vignette.enabled && parameters.vignette.debug != colorCorrectShaderSettings.intDefines["DEBUG_VIGNETTE"]){
				colorCorrectShaderSettings.intDefines["DEBUG_VIGNETTE"] = parameters.vignette.debug;
                colorCorrectShader.setup(colorCorrectShaderSettings);
            }

			if(parameters.vignette.enabled && parameters.vignette.onlyAlpha != colorCorrectShaderSettings.intDefines["ONLY_ALPHA"]){
				colorCorrectShaderSettings.intDefines["ONLY_ALPHA"] = parameters.vignette.onlyAlpha;
				colorCorrectShader.setup(colorCorrectShaderSettings);
			}

			if (parameters.bloom.enabled) 
			{
				// Pass 0: Brightness
				this->fboTemp[0].begin();
				{
					ofClear(0, 255);
					brightnessThresholdShader.begin();
					brightnessThresholdShader.setUniformTexture("tex0", srcTexture, 0);
					brightnessThresholdShader.setUniform1f("bright_threshold", parameters.bloom.brightnessThreshold);
					{
						this->fullQuad.draw();
					}
					brightnessThresholdShader.end();
				}
				this->fboTemp[0].end();

				// Multi-pass Directional Blur.
				auto texel_size = glm::vec2(1. / float(srcTexture.getWidth()), 1. / float(srcTexture.getHeight()));

				auto w0 = gaussian(0.0, 0.0, parameters.bloom.sigma);
				auto w1 = gaussian(1.0, 0.0, parameters.bloom.sigma);
				auto w2 = gaussian(2.0, 0.0, parameters.bloom.sigma);
				auto w3 = gaussian(3.0, 0.0, parameters.bloom.sigma);
				auto w4 = gaussian(4.0, 0.0, parameters.bloom.sigma);
				auto w5 = gaussian(5.0, 0.0, parameters.bloom.sigma);
				auto w6 = gaussian(6.0, 0.0, parameters.bloom.sigma);
				auto w7 = gaussian(7.0, 0.0, parameters.bloom.sigma);
				auto w8 = gaussian(8.0, 0.0, parameters.bloom.sigma);
				auto wn = w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8);

				for (int i = 0; i < parameters.bloom.numPasses; i++)
				{
					// Pass 1: Blur Vertical
					this->fboTemp[1].begin();
					{
						ofClear(0, 255);

						this->blurVertShader.begin();
						this->blurVertShader.setUniformTexture("tex0", this->fboTemp[0].getTexture(), 0);
						this->blurVertShader.setUniform2f("texel_size", texel_size);
						this->blurVertShader.setUniform1f("w0", w0 / wn);
						this->blurVertShader.setUniform1f("w1", w1 / wn);
						this->blurVertShader.setUniform1f("w2", w2 / wn);
						this->blurVertShader.setUniform1f("w3", w3 / wn);
						this->blurVertShader.setUniform1f("w4", w4 / wn);
						this->blurVertShader.setUniform1f("w5", w5 / wn);
						this->blurVertShader.setUniform1f("w6", w6 / wn);
						this->blurVertShader.setUniform1f("w7", w7 / wn);
						this->blurVertShader.setUniform1f("w8", w8 / wn);
						{
							this->fullQuad.draw();
						}
						this->blurVertShader.end();
					}
					this->fboTemp[1].end();

					// Pass 2: Blur Horizontal
					this->fboTemp[0].begin();
					{
						ofClear(0, 255);

						this->blurHorzShader.begin();
						this->blurHorzShader.setUniformTexture("tex0", this->fboTemp[1].getTexture(), 0);
						this->blurHorzShader.setUniform2f("texel_size", texel_size);
						this->blurHorzShader.setUniform1f("w0", w0 / wn);
						this->blurHorzShader.setUniform1f("w1", w1 / wn);
						this->blurHorzShader.setUniform1f("w2", w2 / wn);
						this->blurHorzShader.setUniform1f("w3", w3 / wn);
						this->blurHorzShader.setUniform1f("w4", w4 / wn);
						this->blurHorzShader.setUniform1f("w5", w5 / wn);
						this->blurHorzShader.setUniform1f("w6", w6 / wn);
						this->blurHorzShader.setUniform1f("w7", w7 / wn);
						this->blurHorzShader.setUniform1f("w8", w8 / wn);
						{
							this->fullQuad.draw();
						}
						this->blurHorzShader.end();
					}
					this->fboTemp[0].end();
				}
			}

			// Color Correction.
			dstFbo.begin();
			{
				ofClear(0, 255);

				this->colorCorrectShader.begin();
				this->colorCorrectShader.setUniform1f("exposureBias", parameters.color.exposure);
				this->colorCorrectShader.setUniform1f("gamma", parameters.color.gamma);
				this->colorCorrectShader.setUniform1f("tonemap_type", parameters.color.tonemapping);
				this->colorCorrectShader.setUniform1f("brightness", parameters.color.brightness);
				this->colorCorrectShader.setUniform1f("contrast", parameters.color.contrast);
                this->colorCorrectShader.setUniform1f("inner_vigneting", parameters.vignette.inner);
                this->colorCorrectShader.setUniform1f("outer_vigneting", parameters.vignette.outer);
                this->colorCorrectShader.setUniform1f("vignette_power", parameters.vignette.power);
                this->colorCorrectShader.setUniform1f("ratio", parameters.screenRatio);
                glm::mat3 rot(glm::rotate(glm::radians((float)-parameters.vignette.rotation), glm::vec3(0,0,1)));
                this->colorCorrectShader.setUniformMatrix3f("vignette_rotation", rot);
				if (parameters.bloom.debugBlur)
				{
					this->colorCorrectShader.setUniformTexture("tex0", this->fboTemp[0].getTexture(), 0);
					this->colorCorrectShader.setUniformTexture("blurred1", GL_TEXTURE_2D, 0, 1);
				}
				else
				{
					this->colorCorrectShader.setUniformTexture("tex0", srcTexture, 0);
					if (parameters.bloom.enabled)
					{
						this->colorCorrectShader.setUniformTexture("blurred1", this->fboTemp[0].getTexture(), 1);
					}
					else
					{
						this->colorCorrectShader.setUniformTexture("blurred1", GL_TEXTURE_2D, 0, 1);
					}
				}
				{
					// Draw full-screen quad.
					glBindVertexArray(this->defaultVao);
					glDrawArrays(GL_TRIANGLES, 0, 3);
				}
				this->colorCorrectShader.end();
			}
			dstFbo.end();
		}

		//--------------------------------------------------------------
		void PostEffects::resize(int width, int height)
		{
			if (this->fboSettings.width == width && this->fboSettings.height == height) return;
			
			this->fboSettings.width = width;
			this->fboSettings.height = height;
			this->updateSize();
		}

		//--------------------------------------------------------------
		void PostEffects::updateSize()
		{
			// Re-allocate fbos.
			this->fboSettings.numSamples = 0;
			this->fboSettings.internalformat = GL_RGBA16F;
			this->fboSettings.textureTarget = GL_TEXTURE_2D;
			for (int i = 0; i < 2; ++i)
			{
				this->fboTemp[i].allocate(this->fboSettings);
				//this->fboTemp[i].getTexture().texData.bFlipTexture = true;
			}
        }

		//--------------------------------------------------------------
        std::filesystem::path PostEffects::getShaderPath(const string & shaderFile)
		{
            return GetShadersPath(Module::PostEffects) / shaderFile;
		}
	}
}
