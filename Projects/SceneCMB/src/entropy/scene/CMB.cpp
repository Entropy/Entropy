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
			//this->pool.parameters.base.dropping = m_timeline.isSwitchOn("Dropping");
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

			//if (m_bExportFrames)
			//{
			//	if (m_timeline.getIsPlaying())
			//	{
			//		ofSaveScreen(m_exportPath + ofToString(m_timeline.getCurrentFrame(), 5, '0') + ".png");
			//	}
			//	else
			//	{
			//		m_bExportFrames = false;
			//		endExport();
			//	}
			//}
		}

		//--------------------------------------------------------------
		void CMB::gui(ofxPreset::Gui::Settings & settings)
		{
			ofxPreset::Gui::SetNextWindow(settings);
			ofxPreset::Gui::AddGroup(this->parameters, settings);

			//if (ImGui::Checkbox("Export", &m_bExportFrames))
			//{
			//	if (m_bExportFrames)
			//	{
			//		std::string folderName = ofSystemTextBoxDialog("Save to folder", ofGetTimestampString("%Y%m%d-%H%M%S"));
			//		if (folderName.length())
			//		{
			//			m_exportPath = ofToDataPath("exports/" + folderName + "/");
			//			beginExport();
			//		}
			//	}
			//	else
			//	{
			//		endExport();
			//	}
			//}
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

		////--------------------------------------------------------------
		//void ofApp::beginExport()
		//{
		//	this->pool.setDimensions(m_dimensionExport);
		//	this->pool.setup();

		//	m_timeline.setFrameBased(true);
		//	m_cameraTrack->setTimelineInOutToTrack();
		//	m_cameraTrack->lockCameraToTrack = true;
		//	m_timeline.setCurrentTimeToInPoint();
		//	m_timeline.play();
		//}

		////--------------------------------------------------------------
		//void ofApp::endExport()
		//{
		//	m_timeline.stop();
		//	m_timeline.setFrameBased(false);
		//	m_timeline.setInOutRange(ofRange(0, 1));
		//	m_cameraTrack->lockCameraToTrack = false;

		//	this->pool.setDimensions(m_dimensionEditor);
		//	this->pool.setup();
		//}
	}
}