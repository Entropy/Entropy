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

			this->pool3D.setDimensions(glm::vec3(128.0f));
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

				this->sphereTexture.bind();
				{
					this->sphereGeom.draw();
				}
				this->sphereTexture.unbind();
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