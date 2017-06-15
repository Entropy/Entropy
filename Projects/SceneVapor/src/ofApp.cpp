#include "ofApp.h"
#include "ofxEasing.h"
#include "Helpers.h"

constexpr int appfps = 30;


//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(appfps));
    ofBackground(ofColor::black);
	//ofSetVerticalSync(false);

	//m_sequenceRamses.setup("RAMSES_time_sequence/", 338, 346);
	//m_sequenceRamses.setup("RAMSES_HDF5_data/", 0, 0);
//	m_sequenceRamses.setupRemote("sftp://entropy:$entr0py$@login7.sciama.icg.port.ac.uk:downloads/lvl22_hdf5_1024",
//								 "/media/arturo/elements/entropy/vapor_download_tests", 333, 333);

	m_sequenceRamses.setup("/media/arturo/elements/entropy/vapor", 333, 421);
	m_sequenceRamses.loadFrame(0);


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
	ttf.load("Kontrapunkt Bob, Light", 12, true, true, 72);
	m_gui.setup(parameters);
	m_gui.add(renderer.parameters);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density min").setUpdateOnReleaseOnly(true);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density max").setUpdateOnReleaseOnly(true);

	// Setup timeline.
	m_timeline.setup();
	m_timeline.setLoopType(OF_LOOP_NONE);
	m_timeline.setFrameRate(60.0f);
	m_timeline.setDurationInSeconds(360);


	listeners.push_back(m_showOctreeAnimation.newListener([this](bool&show){
		constexpr size_t num_clusters = 4;
		constexpr double cluster_offset = 0.3;
		if(show){
			rangesH.clear();
			rangesV.clear();
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationStart = m_timeline.getCurrentTime();
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

			BoundingBox fustrum = BoundingBox::fromMinMax(glm::vec3(-0.2,-0.05,-0.001), glm::vec3(0.2,0.05,0.001));
			auto distanceFunction = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshH.getVertices()[i1];
				auto & v2 = octreeAnimationMeshH.getVertices()[i2];
				auto inBox = int((!fustrum.inside(v1) && !fustrum.inside(v2)))		* 1000000000000000.;
				auto zDistanceToCenter = std::max(abs(v1.z), abs(v2.z))				* 1000.;
				auto length = (1-glm::distance(v1,v2))								* 1.;
//				auto verticalDistanceToCenter = std::max(abs(v1.y), abs(v2.y))		* 1000.;
				// auto horizontalDistance = 1. - std::max(v1.x, v2.x) / 0.5;
				return inBox + zDistanceToCenter + length;// + verticalDistanceToCenter; // + horizontalDistance;
			};


			std::sort(indicesH.begin(), indicesH.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return distanceFunction(line1) < distanceFunction(line2);
			});

			std::sort(indicesV.begin(), indicesV.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return distanceFunction(line1) < distanceFunction(line2);
			});

			octreeAnimationMeshH.getIndices().clear();
			for(size_t cluster=0;cluster<num_clusters;cluster++){
				for(size_t i=cluster;i<indicesH.size();i+=num_clusters){
					auto & pair = indicesH[i];
					octreeAnimationMeshH.getIndices().push_back(pair.first);
					octreeAnimationMeshH.getIndices().push_back(pair.second);
				}
			}

			octreeAnimationMeshV.getIndices().clear();
			for(size_t cluster=0;cluster<num_clusters;cluster++){
				for(size_t i=cluster;i<indicesV.size();i+=num_clusters){
					auto & pair = indicesV[i];
					octreeAnimationMeshV.getIndices().push_back(pair.first);
					octreeAnimationMeshV.getIndices().push_back(pair.second);
				}
			}

			for(size_t i=0;i<num_clusters;i++){
				auto size = octreeAnimationMeshH.getIndices().size()/num_clusters;
				auto begin = i*size;
				auto end = begin + size;
				rangesH.emplace_back(octreeAnimationStart + cluster_offset * i, begin, end);
			}
			if(rangesH.back().endIndex < octreeAnimationMeshH.getIndices().size()-1){
				rangesH.emplace_back(octreeAnimationStart, rangesH.back().endIndex, octreeAnimationMeshH.getIndices().size());
			}

			for(size_t i=0;i<num_clusters;i++){
				auto size = octreeAnimationMeshV.getIndices().size()/num_clusters;
				auto begin = i*size;
				auto end = begin + size;
				rangesV.emplace_back(octreeAnimationStart + cluster_offset * (i + num_clusters), begin, end);
			}
			if(rangesV.back().endIndex < octreeAnimationMeshV.getIndices().size()-1){
				rangesV.emplace_back(octreeAnimationStart, rangesV.back().endIndex, octreeAnimationMeshV.getIndices().size());
			}


			octreeAnimationVboH.setMesh(octreeAnimationMeshH, GL_STATIC_DRAW);
			octreeAnimationVboV.setMesh(octreeAnimationMeshV, GL_STATIC_DRAW);
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

	autoMode.ownListener([this](bool &){
		autoStartFrame = ofGetFrameNum();
	});


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
	renderer.setup(1);
	renderer.resize(fboSettings.width, fboSettings.height);

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

#ifdef OFX_VIDEORECORDER
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
#endif
}

//--------------------------------------------------------------
void ofApp::update()
{
	if (m_bSyncPlayback)
	{
		m_sequenceRamses.setFrame(m_timeline.getCurrentFrame());
	}else if(autoMode){

		auto frames = ofGetFrameNum() - autoStartFrame;
		auto time = frames / double(appfps);
		int animation_frames = time * animationFps;
		if(m_sequenceRamses.getCurrentFrame() != animation_frames){
			m_sequenceRamses.setFrame(animation_frames);
		}
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
		auto animate = [&](ofMesh & octreeAnimationMesh, Range & range, float octreeTotalDistance){
			if(range.index+1 < range.endIndex){
				auto now = m_timeline.getCurrentTime();
				auto nextIndex = range.index + 1;
				auto i1 = octreeAnimationMesh.getIndices()[range.index];
				auto i2 = octreeAnimationMesh.getIndices()[nextIndex];
				auto p = octreeAnimationMesh.getVertices()[i1];
				auto nextP = octreeAnimationMesh.getVertices()[i2];
				auto distance = glm::distance(p,nextP);
				auto segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
				auto elapsed = now - range.startTime;
				auto pct = elapsed / segmentDuration;


				float factor;
				{
					auto pctEasing = ofxeasing::map_clamp(pct, 0, 1, 0, 1, ofxeasing::sine::easeOut);
					auto pctP = glm::lerp(p, nextP, float(pctEasing));
					factor = ofxeasing::map(pctEasing, 0, 1, 10, 1, ofxeasing::exp::easeIn);
					auto c = octreeAnimationMesh.getColors()[i1];
					c.a *= factor;
					if(range.lines.getVertices().empty()){
						range.lines.addVertex(p);
						range.lines.addVertex(pctP);
						range.lines.addColor({1,m_alphaInitial});
						range.lines.addColor({1,m_alphaInitial});
						range.linesIndex.push_back(range.index);
						range.linesIndex.push_back(range.index);
					}else{
						range.lines.addVertex(range.lines.getVertices().back());
						range.lines.addVertex(pctP);
						range.lines.addColor({1,m_alphaInitial});
						range.lines.addColor({1,m_alphaInitial});
						range.linesIndex.push_back(range.index);
						range.linesIndex.push_back(range.index);
					}
				}
				while(pct>1 && range.index+3 < octreeAnimationMesh.getNumIndices()){
					range.startTime += segmentDuration;
					range.index += 2;
					nextIndex = range.index + 1;
					i1 = octreeAnimationMesh.getIndices()[range.index];
					i2 = octreeAnimationMesh.getIndices()[nextIndex];
					p = octreeAnimationMesh.getVertices()[i1];
					nextP = octreeAnimationMesh.getVertices()[i2];
					distance = glm::distance(p,nextP);
					segmentDuration = distance * m_octreeAnimationDuration / octreeTotalDistance;
					elapsed = now - range.startTime;
					pct = elapsed / segmentDuration;


					{
						auto pctEasing = ofxeasing::map_clamp(pct, 0, 1, 0, 1, ofxeasing::sine::easeOut);
						auto pctP = glm::lerp(p, nextP, float(pctEasing));
						auto c = octreeAnimationMesh.getColors()[i1];
						c.a *= factor;
						range.lines.addVertex(p);
						range.lines.addVertex(pctP);
						range.lines.addColor({1,m_alphaInitial});
						range.lines.addColor({1,m_alphaInitial});
						range.linesIndex.push_back(range.index);
						range.linesIndex.push_back(range.index);
					}
				}

//				size_t i = 0;
//				for(; range.lines.getColors()[i].a < octreeAnimationMesh.getColors()[i].a; i++){}
//				if(i>0){
//					range.lines.getColors().erase(range.lines.getColors().begin(), range.lines.getColors().begin() + i);
//					range.lines.getVertices().erase(range.lines.getVertices().begin(), range.lines.getVertices().begin() + i);
//				}

	//			range.lines.getColors().resize(range.lines.getVertices().size());
	//			for(size_t i=0; i < range.lines.getNumColors(); i++){
	//				auto pct = i / double(range.lines.getNumColors());
	//				auto alpha = ofxeasing::map(pct, 0, 1, 0.2, 1, ofxeasing::exp::easeIn);
	//				range.lines.getColors()[i] = ofFloatColor(1, alpha);
	//			}

				range.index = ofClamp(range.index, range.startIndex, range.endIndex - 1);

			}

			for(size_t i=0; i<range.lines.getColors().size(); i++){
				auto & c = range.lines.getColors()[i];
				auto ii = range.linesIndex[i];
				auto i1 = octreeAnimationMesh.getIndices()[ii];
				auto & original = octreeAnimationMesh.getColors()[i1];
				if(c.a > original.a){
					c.a *= m_alphaFilter;
				}
				if(c.a < original.a){
					c.a = original.a;
				}
			}
		};

		for(auto & range: rangesH){
			animate(octreeAnimationMeshH, range, octreeTotalDistanceH);
		}
		for(auto & range: rangesV){
			animate(octreeAnimationMeshV, range, octreeTotalDistanceH);
		}
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
//		for(auto & range: rangesH){
//			octreeAnimationVboH.setMesh(range.lines, GL_STATIC_DRAW);
//			renderer.draw(octreeAnimationVboH, 0, range.lines.getNumVertices(), GL_LINES, m_camera);
//		}
//		for(auto & range: rangesV){
//			octreeAnimationVboH.setMesh(range.lines, GL_STATIC_DRAW);
//			renderer.draw(octreeAnimationVboH, 0, range.lines.getNumVertices(), GL_LINES, m_camera);
//		}
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		for(auto & range: rangesH){
			range.lines.draw();
			cout << range.lines.getVertices().size() << endl;
			//octreeAnimationVboH.drawElements(GL_LINES, range.index - range.startIndex, range.startIndex);
		}
		for(auto & range: rangesV){
			range.lines.draw();
			cout << range.lines.getVertices().size() << endl;
			//octreeAnimationVboV.drawElements(GL_LINES, range.index - range.startIndex, range.startIndex);
		}
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
		if(m_timeline.getCurrentTime()>m_timeline.getOutTimeInSeconds() || m_timeline.getCurrentTime()>m_timeline.getDurationInSeconds()){
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
