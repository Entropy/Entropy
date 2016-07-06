#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
    ofBackground(ofColor::black);

    m_scale = 1024.0;

	//m_sequenceRamses.setup("RAMSES_sequence/", 338, 346);
	m_sequenceRamses.setup("RAMSES_HDF5_data/", 0, 0);
	m_sequenceRamses.loadFrame(0);

	// Setup timeline.
	m_timeline.setup();
	m_timeline.setLoopType(OF_LOOP_NONE);
	m_timeline.setFrameRate(30.0f);
	m_timeline.setDurationInSeconds(10);

	m_cameraTrack = new ofxTLCameraTrack();
	m_cameraTrack->setCamera(m_camera);
	m_timeline.addTrack("Camera", m_cameraTrack);

	m_cameraTrack->lockCameraToTrack = false;
	//m_timeline.play();

	m_bSyncPlayback = false;
	m_bExportFrames = false;

    m_bGuiVisible = true;
	//m_camera.disableInertia();
	m_camera.setDistance(2048);

	fullQuadFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F);
	fxaaShader.load("shaders/vert_full_quad.glsl", "shaders/frag_fxaa.glsl");
	std::vector<glm::vec3> vertices = {{ -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 }, { -1, 1, 0 }};
	std::vector<glm::vec2> texcoords = {{ 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 }};
	fullQuad.addVertices(vertices);
	fullQuad.addTexCoords(texcoords);
	fullQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);

	ofSetFrameRate(60);
	ofEnablePointSprites();
	z = 0;
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (m_bSyncPlayback)
	{
		m_sequenceRamses.setFrame(m_timeline.getCurrentFrame());
	}
	m_sequenceRamses.update();

    if (m_bMouseOverGui) 
	{
		m_camera.disableMouseInput();
    }
    else 
	{
		m_camera.enableMouseInput();
    }
	m_bMouseOverGui = false;
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    cam.setNearClip(0);
//    cam.setFarClip(FLT_MAX);

	if(m_doFXAA){
		fullQuadFbo.begin();
		ofClear(0,255);
		m_camera.begin();
		m_sequenceRamses.drawTexture(m_scale);
		m_camera.end();
		fullQuadFbo.end();

		fxaaShader.begin();
		fxaaShader.setUniformTexture("tex0", fullQuadFbo.getTexture(), 0);
		fullQuad.draw();
		fxaaShader.end();
	}else{
		m_camera.begin();
		m_sequenceRamses.drawTexture(m_scale);
		m_camera.end();
	}

	m_camera.begin();
	{
		/*if(m_showOctree){
			ofNoFill();
			m_sequenceRamses.drawOctree(m_scale);
			ofFill();
		}else{
			m_sequenceRamses.draw(m_scale);
		}*/
		ofNoFill();
		ofSetColor(255,255);
        ofDrawBox(0, 0, 0, m_scale, m_scale, m_scale);
		ofFill();
        
        ofDrawAxis(20);
    }
	m_camera.end();

	if (m_bExportFrames)
	{
		if (m_timeline.getIsPlaying())
		{
			ofSaveScreen(m_exportPath + ofToString(m_timeline.getCurrentFrame(), 5, '0') + ".png");
		}
		else
		{
			m_bExportFrames = false;
			m_timeline.setFrameBased(false);
		}
	}

    if (m_bGuiVisible) 
	{
        m_bMouseOverGui = imGui();

		m_timeline.setOffset(ofVec2f(0.0, ofGetHeight() - m_timeline.getHeight()));
		m_timeline.draw();

		m_bMouseOverGui != m_timeline.getDrawRect().inside(ofGetMouseX(), ofGetMouseY());
    }
}

//--------------------------------------------------------------
bool ofApp::imGui()
{
	static const int kGuiMargin = 10;

	bool bMouseOverGui = false;
	m_gui.begin();
	{
		ofVec2f windowPos(kGuiMargin, kGuiMargin);
		ofVec2f windowSize = ofVec2f(0);

		ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
		ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
		if (ImGui::Begin("App", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) 
		{
			ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

			if (ImGui::CollapsingHeader("World", nullptr, true, true))
			{
				ImGui::SliderFloat("Scale", &m_scale, 1.0f, 2048.0f);
			}

			if (ImGui::CollapsingHeader("Playback", nullptr, true, true))
			{
				if (ImGui::Checkbox("Sync Timeline", &m_bSyncPlayback))
				{
					if (m_bSyncPlayback)
					{
						m_timeline.stop();
						m_timeline.setCurrentFrame(0);
						m_timeline.setFrameRate(m_sequenceRamses.getFrameRate());
						m_timeline.setDurationInFrames(m_sequenceRamses.getTotalFrames());
					}
				}

				if (ImGui::Checkbox("Export", &m_bExportFrames))
				{
					if (m_bExportFrames)
					{
						std::string folderName = ofSystemTextBoxDialog("Save to folder", ofGetTimestampString("%Y%m%d-%H%M%S"));
						if (folderName.length())
						{
							m_exportPath = ofToDataPath("exports/" + folderName + "/");

							m_timeline.setCurrentFrame(0);
							m_timeline.setFrameBased(true);
							m_timeline.play();
						}
					}
					else
					{
						m_timeline.stop();
						m_timeline.setFrameBased(false);
					}
				}

				/*if(ImGui::Checkbox("GL debug", &m_glDebug)){
					if(m_glDebug){
						ofEnableGLDebugLog();
					}else{
						ofDisableGLDebugLog();
					}
				}*/

				ImGui::Checkbox("FXAA", &m_doFXAA);
				ImGui::Checkbox("Show octree", &m_showOctree);
			}

			windowSize = ImGui::GetWindowSize();
			ImGui::End();
		}

		ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
		bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());

		windowPos.y += windowSize.y + kGuiMargin;
		bMouseOverGui |= m_sequenceRamses.imGui(windowPos, windowSize);
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

		case OF_KEY_UP:
			z++;
		break;

		case OF_KEY_DOWN:
			z--;
		break;

        default:
            break;
    }
	z = ofClamp(z, 0, 1023);
	cout << z << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
