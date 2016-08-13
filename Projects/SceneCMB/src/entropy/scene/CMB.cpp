#include "CMB.h"

namespace entropy
{
	namespace scene
	{
		//--------------------------------------------------------------
		CMB::CMB()
			: Base()
		{
			ENTROPY_SCENE_SETUP_LISTENER;
		}
		
		//--------------------------------------------------------------
		CMB::~CMB()
		{

		}

		//--------------------------------------------------------------
		void CMB::setup()
		{
			ENTROPY_SCENE_EXIT_LISTENER;
			ENTROPY_SCENE_RESIZE_LISTENER;
			ENTROPY_SCENE_UPDATE_LISTENER;
			ENTROPY_SCENE_DRAW_BACK_LISTENER;
			ENTROPY_SCENE_DRAW_WORLD_LISTENER;
			ENTROPY_SCENE_DRAW_FRONT_LISTENER;
			ENTROPY_SCENE_GUI_LISTENER;
			ENTROPY_SCENE_SERIALIZATION_LISTENERS;

			// Add the pool parameters to the group.
			this->parameters.add(this->pool.parameters);

			this->pool.setDimensions(glm::vec3(512.0f));
			this->pool.setup();
		}
		
		//--------------------------------------------------------------
		void CMB::exit()
		{

		}

		//--------------------------------------------------------------
		void CMB::resize(ofResizeEventArgs & args)
		{
#if defined(COMPUTE_GL_2D) || defined(COMPUTE_CL_2D)
			this->pool.restartSimulation = true;
#endif
		}

		//--------------------------------------------------------------
		void CMB::update(double & dt)
		{
			this->pool.update();
		}

		//--------------------------------------------------------------
		void CMB::drawBack()
		{

		}
		
		//--------------------------------------------------------------
		void CMB::drawWorld()
		{
#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			this->drawPool();
#endif
		}

		//--------------------------------------------------------------
		void CMB::drawFront()
		{
#if defined(COMPUTE_GL_2D) || defined(COMPUTE_CL_2D)
			this->drawPool();
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
		void CMB::serialize(nlohmann::json & json)
		{

		}
		
		//--------------------------------------------------------------
		void CMB::deserialize(const nlohmann::json & json)
		{

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