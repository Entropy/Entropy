#include "ofApp.h"
#include "GLError.h"

namespace ent
{
    //--------------------------------------------------------------
    void ofApp::setup()
    {
        ofSetLogLevel(OF_LOG_VERBOSE);
        //ofSetVerticalSync(false);

		m_backgroundColor = ofColor::black;
        m_tintColor = ofColor::white;

#ifdef COMPUTE_GL_2D
		m_dimensionEditor = ofVec3f(ofGetWidth(), ofGetHeight(), 1.0f);
		m_dimensionExport = m_dimensionEditor;
#elif defined(COMPUTE_GL_3D)
		m_dimensionEditor = ofVec3f(128.0f);
		m_dimensionExport = ofVec3f(384.0f);
#elif defined(COMPUTE_CL_2D)
		m_dimensionEditor = ofVec3f(ofGetWidth(), ofGetHeight(), 1.0f);
		m_dimensionExport = m_dimensionEditor; 
#elif defined(COMPUTE_CL_3D)
		m_dimensionEditor = ofVec3f(192.0f);
		m_dimensionExport = ofVec3f(384.0f);
#endif

		m_cmbScene.setDimensions(m_dimensionEditor);
		m_cmbScene.setup();

#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
		m_camera.setNearClip(0.0f);
		m_camera.setDistance(1000.0f);
#else
		// TODO: Ortho cam
#endif

		// Setup timeline.
		m_timeline.setup();
		m_timeline.setLoopType(OF_LOOP_NONE);
		m_timeline.setFrameRate(30.0f);
		m_timeline.setDurationInSeconds(20);

		m_cameraTrack = new ofxTLCameraTrack();
		m_cameraTrack->setCamera(m_camera);
		m_timeline.addTrack("Camera", m_cameraTrack);
		m_timeline.addSwitches("Dropping");

		m_cameraTrack->lockCameraToTrack = false;
		//m_timeline.play();

		m_bExportFrames = false;

        m_bGuiVisible = true;
    }

    //--------------------------------------------------------------
	void ofApp::update()
	{
		m_cmbScene.m_bDropping = m_timeline.isSwitchOn("Dropping");
		m_cmbScene.update();
	}

    //--------------------------------------------------------------
    void ofApp::draw()
    {
        ofBackground(m_backgroundColor);

        ofPushStyle();
		{
			ofEnableAlphaBlending();
			ofDisableDepthTest(); 
			
			ofSetColor(m_tintColor);

#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			m_camera.begin();
#endif
			{
				m_cmbScene.draw();
			}
#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			m_camera.end();
#endif
		}
        ofPopStyle();

		if (m_bExportFrames)
		{
			if (m_timeline.getIsPlaying())
			{
				ofSaveScreen(m_exportPath + ofToString(m_timeline.getCurrentFrame(), 5, '0') + ".png");
			}
			else
			{
				m_bExportFrames = false;
				endExport();
			}
		}

        if (m_bGuiVisible) 
		{
			m_bMouseOverGui = imGui();

			m_timeline.setOffset(ofVec2f(0.0, ofGetHeight() - m_timeline.getHeight()));
			m_timeline.draw();

			m_bMouseOverGui |= m_timeline.getDrawRect().inside(ofGetMouseX(), ofGetMouseY());
        }
		else
		{
			m_bMouseOverGui = false;
		}

		if (m_bMouseOverGui)
		{
			m_camera.disableMouseInput();
		}
		else
		{
			m_camera.enableMouseInput();
		}
	}

	//--------------------------------------------------------------
	void ofApp::beginExport()
	{
		m_cmbScene.setDimensions(m_dimensionExport);
		m_cmbScene.setup();

		m_timeline.setFrameBased(true);
		m_cameraTrack->setTimelineInOutToTrack();
		m_cameraTrack->lockCameraToTrack = true;
		m_timeline.setCurrentTimeToInPoint();
		m_timeline.play();
	}
	
	//--------------------------------------------------------------
	void ofApp::endExport()
	{
		m_timeline.stop();
		m_timeline.setFrameBased(false);
		m_timeline.setInOutRange(ofRange(0, 1));
		m_cameraTrack->lockCameraToTrack = false;

		m_cmbScene.setDimensions(m_dimensionEditor);
		m_cmbScene.setup();
	}

	//--------------------------------------------------------------
	bool ofApp::imGui()
	{
		static const int kGuiMargin = 10;

		bool bMouseOverGui = false;
		m_gui.begin();
		{
			ofVec2f windowPos(kGuiMargin, kGuiMargin);
			ofVec2f windowSize = ofVec2f::zero();

			ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
			ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
			if (ImGui::Begin("CMB", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);
				
				ImGui::Checkbox("Restart", &m_cmbScene.m_bRestart);

				ImGui::ColorEdit3("Background Color", &m_backgroundColor[0]);
				ImGui::ColorEdit3("Tint Color", &m_tintColor[0]);
				ImGui::ColorEdit3("Drop Color", &m_cmbScene.m_dropColor[0]);

				ImGui::SliderInt("Drop Rate", &m_cmbScene.m_dropRate, 1, 60);
				ImGui::SliderFloat("Damping", &m_cmbScene.m_damping, 0.0f, 1.0f);
				ImGui::SliderFloat("Radius", &m_cmbScene.m_radius, 1.0f, 60.0f);
				ImGui::SliderFloat("Ring Size", &m_cmbScene.m_ringSize, 0.0f, 5.0f);

				if (ImGui::Checkbox("Export", &m_bExportFrames))
				{
					if (m_bExportFrames)
					{
						std::string folderName = ofSystemTextBoxDialog("Save to folder", ofGetTimestampString("%Y%m%d-%H%M%S"));
						if (folderName.length())
						{
							m_exportPath = ofToDataPath("exports/" + folderName + "/");
							beginExport();
						}
					}
					else
					{
						endExport();
					}
				}

				windowSize.set(ImGui::GetWindowSize());
				ImGui::End();
			}

			ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
			bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());
		}
		m_gui.end();

		return bMouseOverGui;
	}

    //--------------------------------------------------------------
    void ofApp::keyPressed(int key)
    {
        switch (key) 
		{
            case '`':
				m_bGuiVisible ^= 1;
                break;

            case OF_KEY_TAB:
                ofToggleFullscreen();
                break;

			case 'L':
				m_cameraTrack->lockCameraToTrack ^= 1;
				break;

			case 'T':
				m_cameraTrack->addKeyframe();
				break;

            default:
                break;
        }
    }

    //--------------------------------------------------------------
    void ofApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void ofApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void ofApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void ofApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void ofApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void ofApp::windowResized(int w, int h)
    {
        m_cmbScene.m_bRestart = true;
    }

    //--------------------------------------------------------------
    void ofApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void ofApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
