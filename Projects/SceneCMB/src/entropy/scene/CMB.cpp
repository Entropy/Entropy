#include "CMB.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		CMB::CMB()
			: Base()
		{}
		
		//--------------------------------------------------------------
		CMB::~CMB()
		{

		}

		//--------------------------------------------------------------
		void CMB::init()
		{
			// Add the pool parameters to the group.
			this->parameters.add(this->pool.parameters);

			// Add the sphere parameters to the group.
			this->parameters.add(this->sphereGeom.parameters);

			this->pool.setDimensions(glm::vec3(128.0f));
			this->pool.setup();

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
		}

		//--------------------------------------------------------------
		void CMB::setup()
		{}

		//--------------------------------------------------------------
		void CMB::resizeBack(ofResizeEventArgs & args)
		{
#if defined(COMPUTE_GL_2D) || defined(COMPUTE_CL_2D)
			this->pool.restartSimulation = true;
#endif
		}

		//--------------------------------------------------------------
		void CMB::update(double dt)
		{
			this->pool.update();
		}
		
		//--------------------------------------------------------------
		void CMB::drawBackWorld()
		{
#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			ofEnableDepthTest();
			this->drawPool();
			ofEnableDepthTest();
			//ofDisableDepthTest();
#endif
			
			//glEnable(GL_CULL_FACE);
			//glCullFace(GL_BACK);
			this->sphereTexture.bind();
			{
				this->sphereGeom.draw();
			}
			this->sphereTexture.unbind();
			//glDisable(GL_CULL_FACE);
		}

		//--------------------------------------------------------------
		void CMB::drawBackOverlay()
		{
#if defined(COMPUTE_GL_2D) || defined(COMPUTE_CL_2D)
			this->drawPool();
#endif
		}

		//--------------------------------------------------------------
		void CMB::drawFrontWorld()
		{
#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			ofEnableDepthTest();
			this->drawPool();
			ofEnableDepthTest();
			//ofDisableDepthTest();
#endif
		}

		//--------------------------------------------------------------
		void CMB::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			if (ofxPreset::Gui::BeginWindow(this->parameters.getName().c_str(), settings, true, nullptr))
			{
				ofxPreset::Gui::AddParameter(this->parameters.tintColor);
				ofxPreset::Gui::AddGroup(this->pool.parameters, settings);
				ofxPreset::Gui::AddGroup(this->sphereGeom.parameters, settings);
			}
			ofxPreset::Gui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void CMB::drawPool()
		{
			ofPushStyle();
			{
				ofSetColor(this->parameters.tintColor.get());

				this->pool.draw();
			}
			ofPopStyle();
		}
	}
}