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
			this->pool2D.setup();

			this->pool3D.setDimensions(glm::vec3(256.0f));
			this->pool3D.setup();

			// Init the sphere.
			this->parameters.add(this->sphereGeom.parameters);

			const auto filePath = this->getAssetsPath("images/Planck-CMB-SMICA.tif");
			//const auto filePath = this->getAssetsPath("images/Gaia_star_density_image_log.png");
			ofPixels pixels;
			ofLoadImage(pixels, filePath);
			if (!pixels.isAllocated())
			{
				ofLogError(__FUNCTION__) << "Could not load file at path " << filePath;
			}

			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->sphereTexture.enableMipmap();
				this->sphereTexture.loadData(pixels);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			//this->sphereShader.load("shaders/passthru.vert", "shaders/reveal.frag");
			this->sphereShader.load("shaders/reveal.vert", "shaders/reveal.frag");

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
			this->pool2D.update();
			this->pool3D.update();
		}

		//--------------------------------------------------------------
		void Bubbles::timelineBangFired(ofxTLBangEventArgs & args)
		{
			static const string kResetFlag = "reset";
			if (args.flag.compare(0, kResetFlag.size(), kResetFlag) == 0)
			{
				this->getCamera(render::Layout::Back)->reset();
				this->getCamera(render::Layout::Front)->reset();

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
			ofPushMatrix();
			{
				ofRotateYDeg(this->parameters.sphere.orientation);

				this->sphereShader.begin();
				{
					this->sphereShader.setUniformTexture("uTexColor", this->sphereTexture, 1);
					this->sphereShader.setUniformTexture("uTexMask", this->pool2D.getTexture(), 2);
					this->sphereShader.setUniform2f("uMaskDims", glm::vec2(this->pool2D.getTexture().getWidth(), this->pool2D.getTexture().getHeight()));
					this->sphereShader.setUniform1f("uAlphaBase", this->sphereGeom.alpha);
					this->sphereShader.setUniform1f("uMaskMix", this->parameters.sphere.maskMix);

					this->sphereGeom.draw();
				}
				this->sphereShader.end();
			}
			ofPopMatrix();

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
				if (ofxPreset::Gui::BeginTree(this->pool2D.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->pool2D.runSimulation);
					ImGui::SameLine();
					if (ImGui::Button("Reset Simulation"))
					{
						this->pool2D.resetSimulation = true;
					}

					ofxPreset::Gui::AddParameter(this->pool2D.drawBack);
					ImGui::SameLine();
					ofxPreset::Gui::AddParameter(this->pool2D.drawFront);

					ofxPreset::Gui::AddParameter(this->pool2D.alpha);

					ofxPreset::Gui::AddParameter(this->pool2D.dropColor1);
					ofxPreset::Gui::AddParameter(this->pool2D.dropColor2);
					ofxPreset::Gui::AddParameter(this->pool2D.dropping);
					ofxPreset::Gui::AddParameter(this->pool2D.dropRate);

					ofxPreset::Gui::AddParameter(this->pool2D.rippleRate);

					ofxPreset::Gui::AddParameter(this->pool2D.damping);
					ofxPreset::Gui::AddParameter(this->pool2D.radius);
					ofxPreset::Gui::AddParameter(this->pool2D.ringSize);

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->pool3D.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->pool3D.runSimulation);
					ImGui::SameLine();
					if (ImGui::Button("Reset Simulation"))
					{
						this->pool3D.resetSimulation = true;
					}

					ofxPreset::Gui::AddParameter(this->pool3D.drawBack);
					ImGui::SameLine();
					ofxPreset::Gui::AddParameter(this->pool3D.drawFront);

					ofxPreset::Gui::AddParameter(this->pool3D.alpha);

					ofxPreset::Gui::AddParameter(this->pool3D.dropColor1);
					ofxPreset::Gui::AddParameter(this->pool3D.dropColor2);
					ofxPreset::Gui::AddParameter(this->pool3D.dropping);
					ofxPreset::Gui::AddParameter(this->pool3D.dropRate);

					ofxPreset::Gui::AddParameter(this->pool3D.rippleRate);

					ofxPreset::Gui::AddParameter(this->pool3D.damping);
					ofxPreset::Gui::AddParameter(this->pool3D.radius);
					ofxPreset::Gui::AddParameter(this->pool3D.ringSize);

					static const vector<string> labels{ "Nearest", "Linear" };
					ofxPreset::Gui::AddRadio(this->pool3D.filterMode, labels, 2);
					ofxPreset::Gui::AddParameter(this->pool3D.volumeSize);

					ofxPreset::Gui::EndTree(settings);
				}

				if (ofxPreset::Gui::BeginTree(this->sphereGeom.parameters, settings))
				{
					ofxPreset::Gui::AddParameter(this->sphereGeom.enabled);
					if (this->sphereGeom.enabled)
					{
						ImGui::SameLine();
						ofxPreset::Gui::AddParameter(this->sphereGeom.autoDraw);
					}
					ofxPreset::Gui::AddParameter(this->sphereGeom.alphaBlend);
					ofxPreset::Gui::AddParameter(this->sphereGeom.depthTest);
					static const vector<string> labels{ "None", "Back", "Front" };
					ofxPreset::Gui::AddRadio(this->sphereGeom.cullFace, labels, 3);
					ofxPreset::Gui::AddParameter(this->sphereGeom.color);
					ofxPreset::Gui::AddParameter(this->sphereGeom.alpha);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.maskMix);
					ofxPreset::Gui::AddParameter(this->sphereGeom.radius);
					ofxPreset::Gui::AddParameter(this->sphereGeom.resolution);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcHorz);
					ofxPreset::Gui::AddParameter(this->sphereGeom.arcVert);
					ofxPreset::Gui::AddParameter(this->parameters.sphere.orientation);

					ofxPreset::Gui::EndTree(settings);
				}
			}
			ofxPreset::Gui::EndWindow(settings);
		}
	}
}