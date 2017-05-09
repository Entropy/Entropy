#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
    ofBackground(ofColor::black);

	//m_sequenceRamses.setup("RAMSES_time_sequence/", 338, 346);
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
	//m_camera.disableInertia();
	m_camera.setDistance(5);
	m_camera.setNearClip(0.01);

	ofxGuiSetFont("Fira Code", 11, true, true, 72);
	ttf.load("Fira Code", 11, true, true, 72);
	m_gui.setup(parameters);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density min").setUpdateOnReleaseOnly(true);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density max").setUpdateOnReleaseOnly(true);

	/*fullQuadFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F);
	fxaaShader.load("shaders/vert_full_quad.glsl", "shaders/frag_fxaa.glsl");
	std::vector<glm::vec3> vertices = {{ -1, -1, 0 }, { 1, -1, 0 }, { 1, 1, 0 }, { -1, 1, 0 }};
	std::vector<glm::vec2> texcoords = {{ 0, 1 }, { 1, 1 }, { 1, 0 }, { 0, 0 }};
	fullQuad.addVertices(vertices);
	fullQuad.addTexCoords(texcoords);
	fullQuad.setMode(OF_PRIMITIVE_TRIANGLE_FAN);*/

	ofSetFrameRate(60);
	ofEnablePointSprites();
	//ofEnableBlendMode(OF_BLENDMODE_ADD);
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (m_bSyncPlayback)
	{
		m_sequenceRamses.setFrame(m_timeline.getCurrentFrame());
	}
	m_sequenceRamses.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    cam.setNearClip(0);
//    cam.setFarClip(FLT_MAX);
	m_camera.begin();
	if(m_showOctree){
		ofNoFill();
		m_sequenceRamses.drawOctree(m_scale);
		ofFill();
	}
	if(m_showOctreeDensities){
		m_sequenceRamses.drawOctreeDensities(ttf, m_camera, m_scale);
	}
	if(m_vboTex){
		m_sequenceRamses.draw(m_scale);
	}else{
		m_sequenceRamses.drawTexture(m_scale);
	}
	if(m_showAxis)
	{
		ofNoFill();
		ofSetColor(255,255);
        ofDrawBox(0, 0, 0, m_scale, m_scale, m_scale);
		ofFill();
        
		ofDrawAxis(0.2);
    }
	m_camera.end();

	if (m_bExportFrames)
	{
		if (m_timeline.getIsPlaying())
		{
			ofSaveScreen(m_exportPath.get() + ofToString(m_timeline.getCurrentFrame(), 5, '0') + ".png");
		}
		else
		{
			m_bExportFrames = false;
			m_timeline.setFrameBased(false);
		}
	}

    if (m_bGuiVisible) 
	{

		m_timeline.setOffset(ofVec2f(0.0, ofGetHeight() - m_timeline.getHeight()));
		m_timeline.draw();
		m_gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch (key) 
	{
		case 'g':
            m_bGuiVisible ^= 1;
            break;

        case OF_KEY_TAB:
            ofToggleFullscreen();
            break;

		case 'L':
			m_cameraTrack->lockCameraToTrack ^= 1;
			break;

//		case 'T':
//			m_cameraTrack->addKeyframe();
//			break;

        default:
            break;
	}
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
