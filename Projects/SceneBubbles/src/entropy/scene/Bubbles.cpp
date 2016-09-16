#include "Bubbles.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		Bubbles::Bubbles()
			: Base()
		{}
		
		//--------------------------------------------------------------
		Bubbles::~Bubbles()
		{

		}

		//--------------------------------------------------------------
		void Bubbles::init()
		{
			// Init the pools.
			this->pool2D.setDimensions(glm::vec2(GetCanvasWidth(render::Layout::Front), GetCanvasHeight(render::Layout::Front)));
			this->pool2D.init();

			this->pool3D.setDimensions(glm::vec3(256.0f));
			this->pool3D.init();

			// Init the sphere.
			this->parameters.add(this->sphereGeom.parameters);

			this->loadTextureImage(this->getAssetsPath("images/Planck-CMB-SMICA.png"), this->sphereTexture);

			auto shaderSettings = ofShader::Settings();
			shaderSettings.intDefines["USE_TEX_ARRAY"] = USE_TEX_ARRAY;
			shaderSettings.bindDefaults = true;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/reveal.vert";
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/reveal.frag";
			this->sphereShader.setup(shaderSettings);

			// Init the parameters.
			this->parameters.add(this->pool2D.parameters);
			this->parameters.add(this->pool3D.parameters);
		}

		//--------------------------------------------------------------
		void Bubbles::setup()
		{
			this->pool2D.reset(); 
			this->pool3D.reset();
		}

		//--------------------------------------------------------------
		void Bubbles::resizeFront(ofResizeEventArgs & args)
		{
			this->pool2D.setDimensions(glm::vec2(args.width, args.height));
		}

		//--------------------------------------------------------------
		void Bubbles::update(double dt)
		{
			this->pool2D.update(dt);
			this->pool3D.update(dt);
		}

		//--------------------------------------------------------------
		void Bubbles::timelineBangFired(ofxTLBangEventArgs & args)
		{
			static const string kResetFlag = "reset";
			if (args.flag.compare(0, kResetFlag.size(), kResetFlag) == 0)
			{
				this->getCamera(render::Layout::Back)->reset(false);
				this->getCamera(render::Layout::Front)->reset(false);

				this->pool2D.resetSimulation = true;
				this->pool3D.resetSimulation = true;

				this->timeline->stop();
			}
		}

		//--------------------------------------------------------------
		void Bubbles::drawBackBase()
		{
			if (this->pool2D.drawBack)
			{
				this->pool2D.draw();
			}
		}
		
		//--------------------------------------------------------------
		void Bubbles::drawBackWorld()
		{
			this->sphereShader.begin();
			{
				this->sphereShader.setUniformTexture("uTexColor", this->sphereTexture, 1);
				this->sphereShader.setUniformTexture("uTexMask", this->pool3D.getTexture().texData.textureTarget, this->pool3D.getTexture().texData.textureID, 2);
				this->sphereShader.setUniform3f("uMaskDims", this->pool3D.getDimensions());
				this->sphereShader.setUniform1f("uVolSize", this->pool3D.volumeSize);
				this->sphereShader.setUniform1f("uAlphaBase", this->sphereGeom.alpha);
				this->sphereShader.setUniform1f("uMaskMix", this->parameters.sphere.maskMix);
				this->sphereShader.setUniform4f("uTintColor", this->parameters.sphere.tintColor.get());

				this->sphereGeom.draw();
			}
			this->sphereShader.end();

			if (this->pool3D.drawBack)
			{
				this->pool3D.draw();
			}
		}

		//--------------------------------------------------------------
		void Bubbles::drawFrontBase()
		{
			if (this->pool2D.drawFront)
			{
				this->pool2D.draw();
			}
		}

		//--------------------------------------------------------------
		void Bubbles::drawFrontWorld()
		{
			if (this->pool3D.drawFront)
			{
				this->pool3D.draw();
			}
		}

		//--------------------------------------------------------------
		void Bubbles::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				this->pool2D.gui(settings);
				this->pool3D.gui(settings);

				if (ofxPreset::Gui::BeginTree(this->sphereGeom.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->sphereGeom.enabled);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.tintColor, false);
					static const vector<string> blendLabels{ "Disabled", "Alpha", "Add", "Subtract", "Multiply", "Screen" };
					ofxPreset::Gui::AddRadio(this->sphereGeom.blendMode, blendLabels, 3);
					ofxPreset::Gui::AddParameter(this->sphereGeom.depthTest);
					static const vector<string> cullLabels{ "None", "Back", "Front" };
					ofxPreset::Gui::AddRadio(this->sphereGeom.cullFace, cullLabels, 3);
					ofxPreset::Gui::AddParameter(this->sphereGeom.color);
					ofxPreset::Gui::AddParameter(this->sphereGeom.alpha);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.maskMix);
					ofxPreset::Gui::AddParameter(this->sphereGeom.radius);
					ofxPreset::Gui::AddParameter(this->sphereGeom.resolution);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcHorz);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcVert);
					ofxPreset::Gui::AddParameter(this->sphereGeom.orientation);

					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}
	}
}