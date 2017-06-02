#include "ofApp.h"
#include "ofxEasing.h"
#include "Helpers.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(30));
    ofBackground(ofColor::black);
	//ofSetVerticalSync(false);

	//m_sequenceRamses.setup("RAMSES_time_sequence/", 338, 346);
	//m_sequenceRamses.setup("RAMSES_HDF5_data/", 0, 0);
//	m_sequenceRamses.setupRemote("sftp://entropy:$entr0py$@login7.sciama.icg.port.ac.uk:downloads/lvl22_hdf5_1024",
//								 "/media/arturo/elements/entropy/vapor_download_tests", 333, 333);

	m_sequenceRamses.setup("/media/arturo/elements/entropy/vapor", 333, 421);
	m_sequenceRamses.loadFrame(0);

	// Setup timeline.
	m_timeline.setup();
	m_timeline.setLoopType(OF_LOOP_NONE);
	m_timeline.setFrameRate(60.0f);
	m_timeline.setDurationInSeconds(360);

//	m_cameraTrack = new ofxTLCameraTrack();
//	m_cameraTrack->setCamera(m_camera);
//	m_timeline.addTrack("Camera", m_cameraTrack);

//	m_cameraTrack->lockCameraToTrack = false;
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
			cout << "recreaqting" << endl;
			octreeAnimationLinesH.clear();
			octreeAnimationLinesV.clear();
			octreeAnimationIndexStartH = octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationIndexStartV = octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationMeshH = m_sequenceRamses.getOctreeMesh(m_scale);
			octreeAnimationMeshV = octreeAnimationMeshH;

			std::vector<std::pair<ofIndexType, ofIndexType>> indicesH;
			std::vector<std::pair<ofIndexType, ofIndexType>> indicesV;
			for(size_t i=0;i<octreeAnimationMeshH.getIndices().size(); i+=2){
				auto & i1 = octreeAnimationMeshH.getIndices()[i];
				auto & i2 = octreeAnimationMeshH.getIndices()[i+1];
				auto & v1 = octreeAnimationMeshH.getVertices()[i1];
				auto & v2 = octreeAnimationMeshH.getVertices()[i2];
				auto length = glm::distance(v1,v2);
				auto horizontality = 1. - (abs(v1.y - v2.y) + abs(v1.z - v2.z)) / length;
				if(horizontality>0.01){
					indicesH.emplace_back(i1, i2);
				}else{
					indicesV.emplace_back(i1, i2);
				}
			}
			BoundingBox fustrum = BoundingBox::fromMinMax(glm::vec3(-0.2,-0.1,-0.001), glm::vec3(0.2,0.1,0.001));

			auto distanceFunction = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshH.getVertices()[i1];
				auto & v2 = octreeAnimationMeshH.getVertices()[i2];
				auto inBox = int((!fustrum.inside(v1) || !fustrum.inside(v2)))		* 1000000000.;
				auto zDistanceToCenter = std::max(abs(v1.z), abs(v2.z))				* 1000000.;
				auto verticalDistanceToCenter = std::max(abs(v1.y), abs(v2.y))		* 1000.;
//					auto horizontalDistance = 1. - std::max(v1.x, v2.x) / 0.5;
				return inBox + zDistanceToCenter + verticalDistanceToCenter; // + horizontalDistance;
			};
			std::sort(indicesH.begin(), indicesH.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return distanceFunction(line1) < distanceFunction(line2);
			});
			std::sort(indicesV.begin(), indicesV.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return distanceFunction(line1) < distanceFunction(line2);
			});

			octreeAnimationMeshH.getIndices().clear();
			for(auto & pair: indicesH){
				octreeAnimationMeshH.getIndices().push_back(pair.first);
				octreeAnimationMeshH.getIndices().push_back(pair.second);
			}

			octreeAnimationMeshV.getIndices().clear();
			for(auto & pair: indicesV){
				octreeAnimationMeshV.getIndices().push_back(pair.first);
				octreeAnimationMeshV.getIndices().push_back(pair.second);
			}


			octreeAnimationVboH.setMesh(octreeAnimationMeshH, GL_STATIC_DRAW);
			octreeAnimationVboV.setMesh(octreeAnimationMeshV, GL_STATIC_DRAW);
			octreeAnimationIndexH = 0;
			octreeAnimationIndexV = 0;
			octreeTotalDistanceH = 0;
			for(size_t i=0;i<octreeAnimationMeshH.getIndices().size();i+=2){
				auto i1 = octreeAnimationMeshH.getIndex(i);
				auto i2 = octreeAnimationMeshH.getIndex(i+1);
				auto p1 = octreeAnimationMeshH.getVertex(i1);
				auto p2 = octreeAnimationMeshH.getVertex(i2);
				auto d = glm::distance(p1, p2);
				octreeTotalDistanceH += d;
			}
			octreeTotalDistanceV = 0;
			for(size_t i=0;i<octreeAnimationMeshV.getIndices().size();i+=2){
				auto i1 = octreeAnimationMeshV.getIndex(i);
				auto i2 = octreeAnimationMeshV.getIndex(i+1);
				auto p1 = octreeAnimationMeshV.getVertex(i1);
				auto p2 = octreeAnimationMeshV.getVertex(i2);
				auto d = glm::distance(p1, p2);
				octreeTotalDistanceV += d;
			}
		}
	}));

	octreeAnimationLinesH.setMode(OF_PRIMITIVE_LINES);
	octreeAnimationLinesV.setMode(OF_PRIMITIVE_LINES);

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
	fboSettings.width = entropy::GetSceneWidth() / 2;
	fboSettings.height = entropy::GetSceneHeight() / 2;
	fboSettings.numSamples = 8;
	fboSettings.internalformat = GL_RGBA32F;

	fbo.allocate(fboSettings);

//	fboSettings.width *= 0.8;
//	fboSettings.height *= 0.8;
//	fboSettings.numSamples = 16;
	fboLines.allocate(fboSettings);
	fboLines.begin();
	ofClear(0,0);
	fboLines.end();


	listeners.push_back(m_bExportFrames.newListener([this](bool & record){
		if(record){
			auto path = ofSystemLoadDialog("Record to images:", true);
			if(path.bSuccess){
				auto folderPath = path.getPath();
				ofxTextureRecorder::Settings settings(fbo.getTexture());
				settings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				settings.folderPath = folderPath;
				recorder.setup(settings);
			}else{
				m_bExportFrames = false;
			}
		}else{
			recorder.stop();
		}
	}));


	listeners.push_back(m_bRecordVideo.newListener([this](bool & record){
		if(record){
			auto path = ofSystemSaveDialog("video.mp4", "Record to video:");
			if(path.bSuccess){
				auto videoRecorderPath = path.getPath();
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), 30);
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
	}else if(autoMode){
		m_sequenceRamses.setFrame(m_sequenceRamses.getCurrentFrame() + 1);
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

	if(m_showOctreeAnimation){
		auto animate = [&](ofMesh & octreeAnimationMesh, ofMesh & octreeAnimationLines, float octreeTotalDistance, ofIndexType & octreeAnimationIndex, double & octreeAnimationIndexStart){
			if(octreeAnimationIndex+1 < octreeAnimationMesh.getNumIndices()){
				auto now = m_timeline.getCurrentTime();
				auto nextIndex = octreeAnimationIndex + 1;
				auto i1 = octreeAnimationMesh.getIndices()[octreeAnimationIndex];
				auto i2 = octreeAnimationMesh.getIndices()[nextIndex];
				auto p = octreeAnimationMesh.getVertices()[i1];
				auto nextP = octreeAnimationMesh.getVertices()[i2];
				auto distance = glm::distance(p,nextP);
				auto segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
				auto elapsed = now - octreeAnimationIndexStart;
				auto pct = elapsed / segmentDuration;


				float factor;
				{
					auto pctEasing = ofxeasing::map_clamp(pct, 0, 1, 0, 1, ofxeasing::sine::easeOut);
					auto pctP = glm::lerp(p, nextP, float(pctEasing));
					factor = ofxeasing::map(pctEasing, 0, 1, 10, 1, ofxeasing::exp::easeIn);
					auto c = octreeAnimationMesh.getColors()[i1];
					c.a *= factor;
					if(octreeAnimationLines.getVertices().empty()){
						octreeAnimationLines.addVertex(p);
						octreeAnimationLines.addVertex(pctP);
						octreeAnimationLines.addColor({1,m_alphaInitial});
						octreeAnimationLines.addColor({1,m_alphaInitial});
					}else{
						octreeAnimationLines.addVertex(octreeAnimationLines.getVertices().back());
						octreeAnimationLines.addVertex(pctP);
						octreeAnimationLines.addColor({1,m_alphaInitial});
						octreeAnimationLines.addColor({1,m_alphaInitial});
					}
				}
				while(pct>1 && octreeAnimationIndex+3 < octreeAnimationMesh.getNumIndices()){
					octreeAnimationIndexStart += segmentDuration;
					octreeAnimationIndex += 2;
					nextIndex = octreeAnimationIndex + 1;
					i1 = octreeAnimationMesh.getIndices()[octreeAnimationIndex];
					i2 = octreeAnimationMesh.getIndices()[nextIndex];
					p = octreeAnimationMesh.getVertices()[i1];
					nextP = octreeAnimationMesh.getVertices()[i2];
					distance = glm::distance(p,nextP);
					segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
					elapsed = now - octreeAnimationIndexStart;
					pct = elapsed / segmentDuration;


					{
						auto pctEasing = ofxeasing::map_clamp(pct, 0, 1, 0, 1, ofxeasing::sine::easeOut);
						auto pctP = glm::lerp(p, nextP, float(pctEasing));
						auto c = octreeAnimationMesh.getColors()[i1];
						c.a *= factor;
						octreeAnimationLines.addVertex(p);
						octreeAnimationLines.addVertex(pctP);
						octreeAnimationLines.addColor({1,m_alphaInitial});
						octreeAnimationLines.addColor({1,m_alphaInitial});
					}
				}

				for(auto & c: octreeAnimationLines.getColors()){
					c.a *= m_alphaFilter;
				}

	//			octreeAnimationLines.getColors().resize(octreeAnimationLines.getVertices().size());
	//			for(size_t i=0; i < octreeAnimationLines.getNumColors(); i++){
	//				auto pct = i / double(octreeAnimationLines.getNumColors());
	//				auto alpha = ofxeasing::map(pct, 0, 1, 0.2, 1, ofxeasing::exp::easeIn);
	//				octreeAnimationLines.getColors()[i] = ofFloatColor(1, alpha);
	//			}

			}
		};

		animate(octreeAnimationMeshH, octreeAnimationLinesH, octreeTotalDistanceH, octreeAnimationIndexH, octreeAnimationIndexStartH);
		animate(octreeAnimationMeshV, octreeAnimationLinesV, octreeTotalDistanceV, octreeAnimationIndexV, octreeAnimationIndexStartV);
		octreeAnimationIndexH = ofClamp(octreeAnimationIndexH, 0, octreeAnimationMeshH.getNumIndices()-1);
		octreeAnimationIndexV = ofClamp(octreeAnimationIndexV, 0, octreeAnimationMeshV.getNumIndices()-1);
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
//    cam.setNearClip(0);
//    cam.setFarClip(FLT_MAX);

	float h = float(fbo.getHeight()) *  ofGetWidth() / float(fbo.getWidth());
	m_camera.setControlArea({0,0,float(ofGetWidth()),h});

//	if(m_showOctreeAnimation){
//		fboLines.begin();
//		ofClear(0,0);
//		m_camera.begin();
//		shader.begin();
//		shader.setUniform1f("alpha", m_octreeAlpha);
//		ofSetColor(255);
//		octreeAnimationLines.draw();
//		shader.end();
//		m_camera.end();
//		fboLines.end();
//	}

	fbo.begin();
	ofClear(0,255);
	ofSetColor(255);

	m_camera.begin();
	if(m_showOctreeAnimation){
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		octreeAnimationLinesH.draw();
		octreeAnimationLinesV.draw();
		octreeAnimationVboH.drawElements(GL_LINES, octreeAnimationIndexH, 0);
		octreeAnimationVboV.drawElements(GL_LINES, octreeAnimationIndexV, 0);
		shader.end();
	}
	if(m_showOctree){
		ofNoFill();
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		m_sequenceRamses.drawOctree(m_scale);
		//octreeAnimationVbo.drawElements(GL_LINES, octreeAnimationMesh.getIndices().size(), 0);
		shader.end();
		ofFill();
	}
	m_camera.end();

//	if(m_showOctreeAnimation){
//		fboLines.draw(0,0,fbo.getWidth(),fbo.getHeight());
//	}


	m_camera.begin();
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
	fbo.end();


	fbo.draw(0,0,ofGetWidth(),h);

	if (m_bExportFrames || m_bRecordVideo)
	{
		recorder.save(fbo.getTexture());
		if(m_timeline.getCurrentTime()>m_timeline.getOutTimeInSeconds()){
			recorder.stop();
			ofExit(0);
		}
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

//		case 'L':
//			m_cameraTrack->lockCameraToTrack ^= 1;
//			break;

//		case 'T':
//			m_cameraTrack->addKeyframe();
//			break;

		case 's':
			m_bExportFrames = !m_bExportFrames;
			break;

		case OF_KEY_RIGHT:
			m_sequenceRamses.setFrame(m_sequenceRamses.getCurrentFrame() + 1);
		break;

		case OF_KEY_RETURN:
			autoMode = !autoMode;
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
