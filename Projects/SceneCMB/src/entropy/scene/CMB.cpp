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
		}

		//--------------------------------------------------------------
		void CMB::setup()
		{
			this->pool.setDimensions(glm::vec3(128.0f));
			this->pool.setup();
		}

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