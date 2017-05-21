#include "ofApp.h"
#include "ofxEasing.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
    ofBackground(ofColor::black);
	ofSetVerticalSync(false);

	//m_sequenceRamses.setup("RAMSES_time_sequence/", 338, 346);
	//m_sequenceRamses.setup("RAMSES_HDF5_data/", 0, 0);
	m_sequenceRamses.setupRemote("sftp://entropy:$entr0py$@login7.sciama.icg.port.ac.uk:downloads",
								 "/media/arturo/elements/entropy/vapor_download_tests", 1160, 1160);
	m_sequenceRamses.loadFrame(0);

	// Setup timeline.
	m_timeline.setup();
	m_timeline.setLoopType(OF_LOOP_NONE);
	m_timeline.setFrameRate(60.0f);
	m_timeline.setDurationInSeconds(120);

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

	listeners.push_back(m_showOctreeAnimation.newListener([this](bool&show){
		if(show){
			octreeAnimationIndexStart = octreeAnimationStart = ofGetCurrentTime();
			octreeAnimationMesh = m_sequenceRamses.getOctreeMesh(m_scale);
			octreeAnimationVbo.setMesh(octreeAnimationMesh, GL_STATIC_DRAW);
			octreeAnimationIndex = 0;
			octreeTotalDistance = 0;
			for(size_t i=0;i<octreeAnimationMesh.getIndices().size();i+=2){
				auto i1 = octreeAnimationMesh.getIndex(i);
				auto i2 = octreeAnimationMesh.getIndex(i+1);
				auto p1 = octreeAnimationMesh.getVertex(i1);
				auto p2 = octreeAnimationMesh.getVertex(i2);
				auto d = glm::distance(p1, p2);
				octreeTotalDistance += d;
			}
		}
	}));

//	listeners.push_back(m_exportPath.newListener([this](string & path){
//		if(ofDirectory(path).exists()){
//			ofxTextureRecorder::Settings settings(fbo.getTexture());
//			settings.imageFormat = OF_IMAGE_FORMAT_JPEG;
//			settings.folderPath = path;
//			recorder.setup(settings);
//		}
//	}));

	m_fov = m_camera.getFov();

	m_gui.setTimeline(&m_timeline);

	shader.load("octree.vert.glsl", "octree.frag.glsl");

	ofFbo::Settings fboSettings;
	fboSettings.width = ofGetWidth();
	fboSettings.height = ofGetHeight();
	fboSettings.numSamples = 8;
	fboSettings.internalformat = GL_RGBA32F;

	fbo.allocate(fboSettings);

//	ofxTextureRecorder::Settings settings(fbo.getTexture());
//	settings.imageFormat = OF_IMAGE_FORMAT_JPEG;
//	settings.folderPath = "render";
//	recorder.setup(settings);



	listeners.push_back(m_bRecordVideo.newListener([this](bool & record){
		if(record){
			auto path = ofSystemSaveDialog("video.mp4", "Record to video:");
			if(path.bSuccess){
				auto videoRecorderPath = path.getPath();
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), 60);
				recorderSettings.videoPath = videoRecorderPath;
//				recorderSettings.videoCodec = "libx264";
//				recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				recorderSettings.videoCodec = "prores";
				recorderSettings.extrasettings = "-profile:v 0";
				recorder.setup(recorderSettings);
			}else{
				m_bRecordVideo = false;
			}
		}else{
			recorder.stop();
		}
	}));
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (m_bSyncPlayback)
	{
		m_sequenceRamses.setFrame(m_timeline.getCurrentFrame());
	}
	m_sequenceRamses.update();
	m_camera.setFov(m_fov);

	if(m_timeline.getCurrentTime()>m_orbitStart && m_timeline.getCurrentTime()<m_orbitEnd){
		if(orbitStartDistance==0){
			orbitStartDistance = m_camera.getDistance();
		}
		auto angle = ofxeasing::map_clamp(m_timeline.getCurrentTime(), m_orbitStart, m_orbitEnd, 0, 360, ofxeasing::sine::easeIn);
		auto distance = ofxeasing::map_clamp(m_timeline.getCurrentTime(), m_orbitStart, m_orbitEnd, orbitStartDistance, m_finalRadius, ofxeasing::sine::easeIn);
		m_camera.orbitDeg(angle, 0, distance);
	}else{
		orbitStartDistance = 0;
	}

	if(m_cameraAutoDistance){
		auto fov = m_camera.getFov();
		float eyeY = 1 / (2. * m_cameraAutoDistanceMagnification);
		float halfFov = PI * fov / 360.;
		float theTan = tanf(halfFov);
		float distance = eyeY / theTan;

		m_camera.setDistance(distance);
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    cam.setNearClip(0);
//    cam.setFarClip(FLT_MAX);
	fbo.begin();
	ofClear(0,255);
	m_camera.begin();
	if(m_showOctree){
		ofNoFill();
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		m_sequenceRamses.drawOctree(m_scale);
		shader.end();
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
	if(m_showOctreeAnimation){
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		if(octreeAnimationIndex+1 < octreeAnimationMesh.getNumIndices()){
			auto now = ofGetCurrentTime();
			auto nextIndex = octreeAnimationIndex + 1;
			auto i1 = octreeAnimationMesh.getIndices()[octreeAnimationIndex];
			auto i2 = octreeAnimationMesh.getIndices()[nextIndex];
			auto p = octreeAnimationMesh.getVertices()[i1];
			auto nextP = octreeAnimationMesh.getVertices()[i2];
			auto distance = glm::distance(p,nextP);
			auto segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
			auto elapsed = std::chrono::duration<double>(now - octreeAnimationIndexStart);
			auto pct = elapsed.count() / segmentDuration;
			while(pct>1 && octreeAnimationIndex+3 < octreeAnimationMesh.getNumIndices()){
				octreeAnimationIndexStart += std::chrono::duration<double>(segmentDuration);
				octreeAnimationIndex += 2;
				nextIndex = octreeAnimationIndex + 1;
				i1 = octreeAnimationMesh.getIndices()[octreeAnimationIndex];
				i2 = octreeAnimationMesh.getIndices()[nextIndex];
				p = octreeAnimationMesh.getVertices()[i1];
				nextP = octreeAnimationMesh.getVertices()[i2];
				distance = glm::distance(p,nextP);
				segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
				elapsed = std::chrono::duration<double>(now - octreeAnimationIndexStart);
				pct = elapsed.count() / segmentDuration;
			}
			auto c = octreeAnimationMesh.getColors()[i1];
			pct = ofxeasing::map(pct, 0, 1, 0, 1, ofxeasing::quad::easeOut);
			auto pctP = glm::lerp(p, nextP, float(pct));
			//ofSetColor(c);
			//ofDrawLine(p, pctP);
			ofMesh mesh;
			mesh.setMode(OF_PRIMITIVE_LINES);
			mesh.addVertex(p);
			mesh.addVertex(pctP);
			mesh.addColor(c);
			mesh.addColor(c);
			mesh.draw();
		}
		ofSetColor(255);
		octreeAnimationIndex = ofClamp(octreeAnimationIndex, 0, octreeAnimationMesh.getNumIndices()-1);
		octreeAnimationVbo.drawElements(GL_LINES, octreeAnimationIndex, 0);
		shader.end();
	}
	m_camera.end();
	fbo.end();

	fbo.draw(0,0);

	if (m_bExportFrames || m_bRecordVideo)
	{
		recorder.save(fbo.getTexture());
	}

    if (m_bGuiVisible) 
	{

		m_timeline.setOffset(ofVec2f(0.0, ofGetHeight() - m_timeline.getHeight()));
		if(m_bShowTimeline){
			m_timeline.draw();
		}
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

		case 'T':
			m_cameraTrack->addKeyframe();
			break;

		case 's':
			m_bExportFrames = !m_bExportFrames;
			break;

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
