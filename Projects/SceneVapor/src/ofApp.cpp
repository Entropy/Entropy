#include "ofApp.h"
#include "ofxEasing.h"
#include "Helpers.h"


constexpr int appfps = 60;
constexpr float size_factor = 1;
//--------------------------------------------------------------
void ofApp::setup()
{
#ifdef DEBUG
	ofEnableGLDebugLog();
#endif
	const int width = entropy::GetSceneWidth() * size_factor;
	const int height = entropy::GetSceneHeight() * size_factor;

    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(appfps));
    ofBackground(ofColor::black);
	ofSetVerticalSync(false);

	//m_sequenceRamses.setup("RAMSES_time_sequence/", 338, 346);

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
	m_gui.add(postParameters);
	linesPostParameters.setName("Lines postpo");
	m_gui.add(linesPostParameters);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density min").setUpdateOnReleaseOnly(true);
	m_gui.getGroup("Sequence Ramses").getFloatSlider("density max").setUpdateOnReleaseOnly(true);
	m_gui.minimizeAll();

	// Setup timeline.
	m_timeline.setup();
	m_timeline.setLoopType(OF_LOOP_NONE);
	m_timeline.setFrameRate(appfps);
	m_timeline.setDurationInSeconds(420);


	listeners.push_back(m_showOctreeAnimation.newListener([this](bool&show){
		constexpr size_t num_clusters = 4;
		constexpr double cluster_offset = 0.3;
		if(show){
			rangesH.clear();
			rangesV.clear();
			rangesD.clear();
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationMeshH = m_sequenceRamses.getOctreeMesh(m_scale, 0);
			octreeAnimationMeshD = octreeAnimationMeshV = octreeAnimationMeshH;

			std::vector<std::pair<ofIndexType, ofIndexType>> indicesH;
			std::vector<std::pair<ofIndexType, ofIndexType>> indicesV;
			std::vector<std::pair<ofIndexType, ofIndexType>> indicesD;
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
					auto verticallity = 1. - abs(v1.z - v2.z) / length;
					if(verticallity>0.01){
						indicesV.emplace_back(i1, i2);
					}else{
						indicesD.emplace_back(i1, i2);
					}
				}
			}

			auto mostLeft = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshH.getVertices()[i1];
				auto & v2 = octreeAnimationMeshH.getVertices()[i2];
				return std::min(v1.z, v2.z) * 1000000 + std::min(v1.y, v2.y) * 1000 + std::min(v1.x, v2.x);
			};

			std::sort(indicesH.begin(), indicesH.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return mostLeft(line1) < mostLeft(line2);
			});
			ofMesh newH;
			newH.setMode(OF_PRIMITIVE_LINES);
			std::vector<std::pair<ofIndexType, ofIndexType>> newIndicesH;

			int i = 0;
			for(auto & line: indicesH){
				auto & v1 = octreeAnimationMeshH.getVertices()[line.first];
				auto & v2 = octreeAnimationMeshH.getVertices()[line.second];
				auto c1 = ofFloatColor(m_octreeColor, octreeAnimationMeshH.getColors()[line.first].a);
				auto c2 = ofFloatColor(m_octreeColor, octreeAnimationMeshH.getColors()[line.second].a);

				if(newH.getVertices().empty() || abs(newH.getVertices().back().y - v1.y) > 0.001 || abs(newH.getVertices().back().z - v1.z) > 0.001){
					if(v1.x<v2.x){
						newH.addVertex(v1);
						newH.addVertex(v2);
					}else{
						newH.addVertex(v2);
						newH.addVertex(v1);
					}
					newH.addColor(c1);
					newH.addColor(c2);
					auto i1 = i++;
					auto i2 = i++;
					newH.addIndex(i1);
					newH.addIndex(i2);
					newIndicesH.emplace_back(i1, i2);
				}else{
					newH.getVertices().back().x = std::max(v1.x, v2.x);
				}
			}

			indicesH = newIndicesH;
			octreeAnimationMeshH = newH;



			auto mostTop = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshV.getVertices()[i1];
				auto & v2 = octreeAnimationMeshV.getVertices()[i2];
				return std::min(v1.z, v2.z) * 1000000 + std::min(v1.x, v2.x) * 1000 + std::min(v1.y, v2.y);
			};

			std::sort(indicesV.begin(), indicesV.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return mostTop(line1) < mostTop(line2);
			});
			ofMesh newV;
			newV.setMode(OF_PRIMITIVE_LINES);
			std::vector<std::pair<ofIndexType, ofIndexType>> newIndicesV;

			i = 0;
			for(auto & line: indicesV){
				auto & v1 = octreeAnimationMeshV.getVertices()[line.first];
				auto & v2 = octreeAnimationMeshV.getVertices()[line.second];
				auto c1 = ofFloatColor(m_octreeColor, octreeAnimationMeshV.getColors()[line.first].a);
				auto c2 = ofFloatColor(m_octreeColor, octreeAnimationMeshV.getColors()[line.second].a);

				if(newV.getVertices().empty() || abs(newV.getVertices().back().x - v1.x) > 0.001 || abs(newV.getVertices().back().z - v1.z) > 0.001){
					if(v1.y<v2.y){
						newV.addVertex(v1);
						newV.addVertex(v2);
					}else{
						newV.addVertex(v2);
						newV.addVertex(v1);
					}
					newV.addColor(c1);
					newV.addColor(c2);
					auto i1 = i++;
					auto i2 = i++;
					newV.addIndex(i1);
					newV.addIndex(i2);
					newIndicesV.emplace_back(i1, i2);
				}else{
					newV.getVertices().back().y = std::max(v1.y, v2.y);
				}
			}

			indicesV = newIndicesV;
			octreeAnimationMeshV = newV;


			auto mostFront = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshD.getVertices()[i1];
				auto & v2 = octreeAnimationMeshD.getVertices()[i2];
				return  std::min(v1.x, v2.x) * 1000000 + std::min(v1.y, v2.y) * 1000 + std::min(v1.z, v2.z);
			};

			std::sort(indicesD.begin(), indicesD.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return mostFront(line1) < mostFront(line2);
			});
			ofMesh newD;
			newD.setMode(OF_PRIMITIVE_LINES);
			std::vector<std::pair<ofIndexType, ofIndexType>> newIndicesD;

			i = 0;
			for(auto & line: indicesD){
				auto & v1 = octreeAnimationMeshD.getVertices()[line.first];
				auto & v2 = octreeAnimationMeshD.getVertices()[line.second];
				auto c1 = ofFloatColor(m_octreeColor, octreeAnimationMeshD.getColors()[line.first].a);
				auto c2 = ofFloatColor(m_octreeColor, octreeAnimationMeshD.getColors()[line.second].a);

				if(newD.getVertices().empty() || abs(newD.getVertices().back().x - v1.x) > 0.001 || abs(newD.getVertices().back().y - v1.y) > 0.001){
					if(v1.z<v2.z){
						newD.addVertex(v1);
						newD.addVertex(v2);
					}else{
						newD.addVertex(v2);
						newD.addVertex(v1);
					}
					newD.addColor(c1);
					newD.addColor(c2);
					auto i1 = i++;
					auto i2 = i++;
					newD.addIndex(i1);
					newD.addIndex(i2);
					newIndicesD.emplace_back(i1, i2);
				}else{
					newD.getVertices().back().z = std::max(v1.z, v2.z);
				}
			}

			indicesD = newIndicesD;
			octreeAnimationMeshD = newD;

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

			std::sort(indicesD.begin(), indicesD.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
				return distanceFunction(line1) < distanceFunction(line2);
			});


			octreeAnimationMeshH.getIndices().clear();
			auto begin = 0;
			auto end = 0;
			i = 0;
			for(size_t cluster=0;cluster<num_clusters;cluster++){
				for(size_t i=cluster;i<indicesH.size();i+=num_clusters){
					auto & pair = indicesH[i];
					octreeAnimationMeshH.getIndices().push_back(pair.first);
					octreeAnimationMeshH.getIndices().push_back(pair.second);
					end += 2;
				}
				rangesH.emplace_back(octreeAnimationStart + cluster_offset * i++, begin, end);
				begin = end;
			}

			octreeAnimationMeshV.getIndices().clear();
			begin = 0;
			end = 0;
			i = 0;
			for(size_t cluster=0;cluster<num_clusters;cluster++){
				for(size_t i=cluster;i<indicesV.size();i+=num_clusters){
					auto & pair = indicesV[i];
					octreeAnimationMeshV.getIndices().push_back(pair.first);
					octreeAnimationMeshV.getIndices().push_back(pair.second);
					end += 2;
				}
				rangesV.emplace_back(octreeAnimationStart + cluster_offset * (i++ + num_clusters), begin, end);
				begin = end;
			}

			octreeAnimationMeshD.getIndices().clear();
			begin = 0;
			end = 0;
			i = 0;
			for(size_t cluster=0;cluster<num_clusters;cluster++){
				for(size_t i=cluster;i<indicesD.size();i+=num_clusters){
					auto & pair = indicesD[i];
					octreeAnimationMeshD.getIndices().push_back(pair.first);
					octreeAnimationMeshD.getIndices().push_back(pair.second);
					end += 2;
				}
				rangesD.emplace_back(octreeAnimationStart + cluster_offset * (i++ + num_clusters), begin, end);
				begin = end;
			}


//			octreeAnimationVboH.setMesh(octreeAnimationMeshH, GL_STATIC_DRAW);
//			octreeAnimationVboV.setMesh(octreeAnimationMeshV, GL_STATIC_DRAW);
//			octreeAnimationVboD.setMesh(octreeAnimationMeshD, GL_STATIC_DRAW);
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
			octreeTotalDistanceD = 0;
			for(size_t i=0;i<octreeAnimationMeshD.getIndices().size();i+=2){
				auto i1 = octreeAnimationMeshD.getIndex(i);
				auto i2 = octreeAnimationMeshD.getIndex(i+1);
				auto p1 = octreeAnimationMeshD.getVertex(i1);
				auto p2 = octreeAnimationMeshD.getVertex(i2);
				auto d = glm::distance(p1, p2);
				octreeTotalDistanceD += d;
			}
		}
	}));

	auto initAnimation = [](float now, ofMesh & octreeAnimationMesh, std::vector<Range> & ranges, float & octreeTotalDistance, std::function<bool(glm::vec3&, glm::vec3&)> filter, std::function<float(std::pair<ofIndexType, ofIndexType> & line)> distanceFunction){
		constexpr size_t num_clusters = 40;
		constexpr double cluster_offset = 0;//.3;
		ranges.clear();

		std::vector<std::pair<ofIndexType, ofIndexType>> indices;
		for(size_t i=0;i<octreeAnimationMesh.getIndices().size(); i+=2){
			auto & i1 = octreeAnimationMesh.getIndices()[i];
			auto & i2 = octreeAnimationMesh.getIndices()[i+1];
			auto & v1 = octreeAnimationMesh.getVertices()[i1];
			auto & v2 = octreeAnimationMesh.getVertices()[i2];
			if(filter(v1, v2)){
				indices.emplace_back(i1, i2);
			}
		}

		std::sort(indices.begin(), indices.end(), [&](std::pair<ofIndexType, ofIndexType> & line1, std::pair<ofIndexType, ofIndexType> & line2){
			return distanceFunction(line1) < distanceFunction(line2);
		});

		octreeAnimationMesh.getIndices().clear();
		auto begin = 0;
		auto end = 0;
		auto i = 0;
		for(size_t cluster=0;cluster<num_clusters;cluster++){
			for(size_t i=cluster;i<indices.size();i+=num_clusters){
				auto & pair = indices[i];
				octreeAnimationMesh.getIndices().push_back(pair.first);
				octreeAnimationMesh.getIndices().push_back(pair.second);
				end += 2;
			}
			ranges.emplace_back(now + cluster_offset * i++, begin, end);
			begin = end;
		}


		octreeTotalDistance = 0;
		for(size_t i=0;i<octreeAnimationMesh.getIndices().size();i+=2){
			auto i1 = octreeAnimationMesh.getIndex(i);
			auto i2 = octreeAnimationMesh.getIndex(i+1);
			auto p1 = octreeAnimationMesh.getVertex(i1);
			auto p2 = octreeAnimationMesh.getVertex(i2);
			auto d = glm::distance(p1, p2);
			octreeTotalDistance += d;
		}
	};

	listeners.push_back(m_showOctreeAnimationH.newListener([this, initAnimation](bool&show){
		if(show){
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationMeshH = m_sequenceRamses.getOctreeMesh(m_scale, 3);
			auto filter = [](glm::vec3 & v1, glm::vec3 & v2){
				auto length = glm::distance(v1,v2);
				auto horizontality = 1. - (abs(v1.y - v2.y) + abs(v1.z - v2.z)) / length;
				return horizontality>0.01;
			};

			auto distanceFunction = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshH.getVertices()[i1];
				auto & v2 = octreeAnimationMeshH.getVertices()[i2];
				auto length = glm::distance(v1,v2)					* 1000000.;
				auto x = std::max(abs(v1.x), abs(v2.x))				* 1.;
				return length + x;
			};

			initAnimation(octreeAnimationStart, octreeAnimationMeshH, rangesH, octreeTotalDistanceH, filter, distanceFunction);
		}
	}));


	listeners.push_back(m_showOctreeAnimationV.newListener([this, initAnimation](bool&show){
		if(show){
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationMeshV = m_sequenceRamses.getOctreeMesh(m_scale, 3);
			auto filter = [](glm::vec3 & v1, glm::vec3 & v2){
				auto length = glm::distance(v1,v2);
				auto verticallity = 1. - (abs(v1.x - v2.x) + abs(v1.z - v2.z)) / length;
				return verticallity>0.01;
			};

			auto distanceFunction = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshV.getVertices()[i1];
				auto & v2 = octreeAnimationMeshV.getVertices()[i2];
				auto length = glm::distance(v1,v2)					* 1000000.;
				auto x = std::max(abs(v1.y), abs(v2.y))				* 1.;
				return length + x;
			};

			initAnimation(octreeAnimationStart, octreeAnimationMeshV, rangesV, octreeTotalDistanceV, filter, distanceFunction);
		}
	}));


	listeners.push_back(m_showOctreeAnimationD.newListener([this, initAnimation](bool&show){
		if(show){
			octreeAnimationStart = m_timeline.getCurrentTime();
			octreeAnimationMeshD = m_sequenceRamses.getOctreeMesh(m_scale, 3);
			auto filter = [](glm::vec3 & v1, glm::vec3 & v2){
				auto length = glm::distance(v1,v2);
				auto depth = 1. - (abs(v1.x - v2.x) + abs(v1.y - v2.y)) / length;
				return depth>0.01;
			};

			auto distanceFunction = [&](std::pair<ofIndexType, ofIndexType> & line){
				auto & i1 = line.first;
				auto & i2 = line.second;
				auto & v1 = octreeAnimationMeshD.getVertices()[i1];
				auto & v2 = octreeAnimationMeshD.getVertices()[i2];
				auto length = glm::distance(v1,v2)					* 1000000.;
				auto x = std::max(abs(v1.z), abs(v2.z))				* 1.;
				return length + x;
			};

			initAnimation(octreeAnimationStart, octreeAnimationMeshD, rangesD, octreeTotalDistanceD, filter, distanceFunction);
		}
	}));

	autoMode.ownListener([this](bool &){
		vaporAnimationTime = 0;
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
	fboSettings.width = width;
	fboSettings.height = height;
	fboSettings.numSamples = 8;
	fboSettings.internalformat = GL_RGBA32F;

	fbo.allocate(fboSettings);
	fboPost.allocate(fboSettings);
	fboLines.allocate(fboSettings);
	fboPostLines.allocate(fboSettings);
	fboComposite.allocate(fboSettings);
	fboLines.begin();
	ofClear(0,0);
	fboLines.end();

	renderer.setup(1);
	renderer.resize(fboSettings.width, fboSettings.height);
	renderer.parameters.fogMaxDistance.setMax(10000);

	posteffects.resize(fboSettings.width, fboSettings.height);


	frameInterpolationShader.load("shaders/frameinterpolation.vert.glsl", "shaders/frameinterpolation.frag.glsl");


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
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), appfps);
				recorderSettings.videoPath = videoRecorderPath;
				auto ext = std::filesystem::path(videoRecorderPath).extension().string();
				if(ext==".mov"){
					recorderSettings.videoCodec = "prores";
					recorderSettings.extrasettings = "-profile:v 0";
				}else if(ext == ".hpv"){
					recorderSettings.videoCodec = "cocgy";
				}else{
					recorderSettings.videoCodec = "libx264";
					recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				}
				recorder.setup(recorderSettings);
			}else{
				m_bRecordVideo = false;
			}
		}else{
			recorder.stop();
		}
	}));
#endif

	listeners.push_back(m_gui.savePressedE.newListener([this]{
		auto saveTo = ofSystemSaveDialog("presets/", "save settings");
		if(saveTo.bSuccess){
			auto path = std::filesystem::path(saveTo.getPath());
			auto folder = path.parent_path();
			auto basename = path.stem().filename().string();
			auto extension = ofToLower(path.extension().string());
			auto timelineDir = (folder / (basename + "_timeline")).string();
			if(extension == ".xml"){
				ofXml xml;
				if(std::filesystem::exists(path)){
					xml.load(path);
				}
				ofSerialize(xml, m_gui.getParameter());
				m_timeline.saveTracksToFolder(timelineDir);
				m_timeline.saveStructure(timelineDir);
				xml.save(path);
			}else if(extension == ".json"){
				ofJson json = ofLoadJson(path);
				ofSerialize(json, m_gui.getParameter());
				m_timeline.saveTracksToFolder(timelineDir);
				m_timeline.saveStructure(timelineDir);
				ofSavePrettyJson(path, json);
			}else{
				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
			}
		}
		return true;
	}));


	listeners.push_back(m_gui.loadPressedE.newListener([this]{
		auto loadFrom = ofSystemLoadDialog("load settings",false,"presets");
		if(loadFrom.bSuccess){
			auto path = std::filesystem::path(loadFrom.getPath());
			auto folder = path.parent_path();
			auto basename = path.stem().filename().string();
			auto extension = ofToLower(path.extension().string());
			auto timelineDir = (folder / (basename + "_timeline")).string();
			if(extension == ".xml"){
				ofXml xml;
				xml.load(path);
				ofDeserialize(xml, m_gui.getParameter());
				m_timeline.loadStructure(timelineDir);
				m_timeline.loadTracksFromFolder(timelineDir);
				m_timeline.setOffset(glm::vec2(0, ofGetHeight() - m_timeline.getHeight()));
				m_gui.refreshTimelined(&m_timeline);
			}else
			if(extension == ".json"){
				ofJson json;
				ofFile jsonFile(path);
				jsonFile >> json;
				ofDeserialize(json, m_gui.getParameter());
				m_timeline.loadStructure(timelineDir);
				m_timeline.loadTracksFromFolder(timelineDir);
				m_timeline.setOffset(glm::vec2(0, ofGetHeight() - m_timeline.getHeight()));
				m_gui.refreshTimelined(&m_timeline);
			}else{
				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
			}
		}
		return true;
	}));

	listeners.push_back(m_playbackEagle.newListener([&](bool & play){
		if(play){
			eagleTime = 0;
			eagleFrame = 0;
		}
	}));

	eagleOctree.load(m_sequenceRamses.getOctreeMesh(1,0));
}


//--------------------------------------------------------------
void ofApp::update()
{
	dt = m_timeline.getCurrentTime() - now;
	now = m_timeline.getCurrentTime();
	if (m_bSyncPlayback){
		m_sequenceRamses.setFrame(m_timeline.getCurrentFrame());
	}else if(autoMode){
		vaporAnimationTime += dt;
		float animation_frame = vaporAnimationTime * double(animationFps);
		float animation_frame_loop = ofClamp(animation_frame,0,354-333);//fmod(animation_frame - vaporAnimationLoopStart, vaporAnimationLoopEnd - vaporAnimationLoopStart) + vaporAnimationLoopStart;
//		if(animation_frame_loop<animation_frame){
//			vaporAnimationLoopStart += dt;
//			vaporAnimationLoopEnd += dt;
//		}
//		if(animation_frame > 354 - 333){
//			animation_frame = 354 - 333;
//		}
		if(frameInterpolation){
			m_sequenceRamses.loadFrame(animation_frame_loop, animation_frame_loop, animation_frame_loop+1);
		}else{
			m_sequenceRamses.setFrame(int(animation_frame));
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
		m_camera.orbitDeg(angle, m_latitude, distance);
	}else{
		//m_camera.orbitDeg(m_longitude, m_latitude, m_finalRadius);
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
	m_camera.setNearClip(std::max(m_camera.getDistance() - eagleOctree.getBoxSize(), 0.1f));
	m_camera.setFarClip(m_camera.getDistance() + eagleOctree.getBoxSize() * 4);

	if(m_showEagleOctree){
		auto cameraOffset = glm::lerp(glm::vec3(0), -eagleOctree.getPos() + m_eagleTrackOffset.get(), m_cameraCenterToEagle.get());
		auto distance = m_camera.getDistance();
		m_camera.setTarget(cameraOffset);
		m_camera.setDistance(distance);
		m_camera.setPosition(cameraOffset + glm::vec3(0,0,distance));
		m_camera.lookAt(cameraOffset);
		eagleOctree.generateMesh(m_camera, cameraOffset, {0,0,fbo.getWidth(),fbo.getHeight()});
	}



	if(m_showOctreeAnimation || m_showOctreeAnimationH || m_showOctreeAnimationV || m_showOctreeAnimationD){
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
					factor = ofxeasing::map_clamp(pctEasing, 0, 1, 10, 1, ofxeasing::exp::easeIn);
					auto c = octreeAnimationMesh.getColors()[i1];
					c.a *= factor;
					if(range.lines.getVertices().empty()){
						range.lines.addVertex(p);
						range.lines.addVertex(pctP);
						range.lines.addColor({m_octreeColor,m_alphaInitial});
						range.lines.addColor({m_octreeColor,m_alphaInitial});
						range.linesIndex.push_back(range.index);
						range.linesIndex.push_back(range.index);
					}else{
						range.lines.addVertex(range.lines.getVertices().back());
						range.lines.addVertex(pctP);
						range.lines.addColor({m_octreeColor,m_alphaInitial});
						range.lines.addColor({m_octreeColor,m_alphaInitial});
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
						range.lines.addColor({m_octreeColor,m_alphaInitial});
						range.lines.addColor({m_octreeColor,m_alphaInitial});
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
				const auto & original = octreeAnimationMesh.getColors()[i1];
				if(c.a > original.a){
					c.a *= m_alphaFilter;
				}
				if(c.a < original.a){
					c.a = original.a;
				}
			}
		};

		if(m_showOctreeAnimation || m_showOctreeAnimationH){
			for(auto & range: rangesH){
				animate(octreeAnimationMeshH, range, octreeTotalDistanceH);
			}
		}

		if(m_showOctreeAnimation || m_showOctreeAnimationV){
			for(auto & range: rangesV){
				animate(octreeAnimationMeshV, range, octreeTotalDistanceV);
			}
		}

		if(m_showOctreeAnimation || m_showOctreeAnimationD){
			for(auto & range: rangesD){
				animate(octreeAnimationMeshD, range, octreeTotalDistanceD);
			}
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

	fboLines.begin();
	ofClear(0,255);
	ofSetColor(255);

	if(m_playbackEagle){
		eagleTime += dt;
		eagleFrame += dt * eagleVideoFps;
//		size_t sequence = eagleTime * 12.;
//		float sequencef = eagleTime * 12.;
//		if(sequence>150){
//			sequence = 150;
//			sequencef = 150;
//		}
		if(eagleFrame > 150){
			eagleFrame = 150;
		}
		eagleOctree.loadImage(floor(eagleFrame), eagleImg1);
		eagleOctree.loadImage(ceil(eagleFrame), eagleImg2);
		auto w = eagleImg1.getWidth()*size_factor;
		auto h = eagleImg2.getHeight()*size_factor;
		auto x = (ofGetViewportWidth() - w) / 2.;
		auto y = (ofGetViewportHeight() - h) / 2;
		frameInterpolationShader.begin();
		frameInterpolationShader.setUniformTexture("texture1", eagleImg1, 0);
		frameInterpolationShader.setUniformTexture("texture2", eagleImg2, 1);
		frameInterpolationShader.setUniform1f("pct", eagleFrame - floor(eagleFrame));
		cout << eagleFrame - floor(eagleFrame) << endl;
		frameInterpolationShader.setUniform1f("alpha", eagleVideoAlpha);
		eagleImg1.draw(x,y,w,h);
		frameInterpolationShader.end();
	}
	fboLines.end();

//	if(m_showOctreeAnimation){
//		fboLines.draw(0,0,fbo.getWidth(),fbo.getHeight());
//	}


	fbo.begin();
	ofClear(0,255);

	m_camera.begin();
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	if(m_showOctreeAnimation || m_showOctreeAnimationH || m_showOctreeAnimationV || m_showOctreeAnimationD){
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
		if(m_showOctreeAnimation || m_showOctreeAnimationH){
			ofPushMatrix();
			ofTranslate(m_offsetOctreeH,0,0);
			for(auto & range: rangesH){
				range.lines.draw();
				//cout << range.lines.getVertices().size() << endl;
				//octreeAnimationVboH.drawElements(GL_LINES, range.index - range.startIndex, range.startIndex);
			}
			ofPopMatrix();
		}
		if(m_showOctreeAnimation || m_showOctreeAnimationV){
			ofPushMatrix();
			ofTranslate(m_offsetOctreeV,0,0);
			for(auto & range: rangesV){
				range.lines.draw();
				//cout << range.lines.getVertices().size() << endl;
				//octreeAnimationVboV.drawElements(GL_LINES, range.index - range.startIndex, range.startIndex);
			}
			ofPopMatrix();
		}
		if(m_showOctreeAnimation || m_showOctreeAnimationD){
			ofPushMatrix();
			ofTranslate(m_offsetOctreeD,0,0);
			for(auto & range: rangesD){
				range.lines.draw();
				//cout << range.lines.getVertices().size() << endl;
				//octreeAnimationVboV.drawElements(GL_LINES, range.index - range.startIndex, range.startIndex);
			}
			ofPopMatrix();
		}
		shader.end();
	}
	if(m_showOctree){
		ofNoFill();
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		m_sequenceRamses.drawOctree(m_scale);
		//octreeAnimationMeshH.draw();
		//octreeAnimationVbo.drawElements(GL_LINES, octreeAnimationMesh.getIndices().size(), 0);
		shader.end();
		ofFill();
	}
	if(m_showOctreeH){
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		ofPushMatrix();
		ofTranslate(m_offsetOctreeH,0,0);
		octreeAnimationMeshH.draw();
		ofPopMatrix();
		shader.end();
	}
	if(m_showOctreeV){
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		ofPushMatrix();
		ofTranslate(m_offsetOctreeV,0,0);
		octreeAnimationMeshV.draw();
		ofPopMatrix();
		shader.end();
	}
	if(m_showOctreeD){
		shader.begin();
		shader.setUniform1f("alpha", m_octreeAlpha);
		ofPushMatrix();
		ofTranslate(m_offsetOctreeD,0,0);
		octreeAnimationMeshD.draw();
		ofPopMatrix();
		shader.end();
	}

	ofEnableAlphaBlending();
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

	if(m_eagleMincubeAlpha>0){
		m_camera.begin();
		shader.begin();
		shader.setUniform1f("alpha", m_eagleMincubeAlpha);
		ofPushMatrix();
		ofTranslate({m_eagleTrackRotationCenterX,0,0});
		ofRotateDeg(m_eagleTrackRotation, 0,1,0);
		ofTranslate({-m_eagleTrackRotationCenterX, 0, 0});
		eagleOctree.drawMinCube();
		ofPopMatrix();
		//renderer.draw(eagleOctree.getVbo(), 0, eagleOctree.getNumVertices(), GL_LINES, m_camera);
		shader.end();
		m_camera.end();
	}

	if(m_showEagleOctree){
		m_camera.begin();
		shader.begin();
		shader.setUniform1f("alpha", m_eagleOctreeAlpha);
		ofPushMatrix();
		ofTranslate({m_eagleTrackRotationCenterX,0,0});
		ofRotateDeg(m_eagleTrackRotation, 0,1,0);
		ofTranslate({-m_eagleTrackRotationCenterX, 0, 0});
		eagleOctree.draw();
		ofPopMatrix();
		//renderer.draw(eagleOctree.getVbo(), 0, eagleOctree.getNumVertices(), GL_LINES, m_camera);
		shader.end();
		m_camera.end();
	}
	fbo.end();

	posteffects.process(fbo.getTexture(), fboPost, postParameters);
	//posteffects.process(fboLines.getTexture(), fboPostLines, linesPostParameters);

//	fboComposite.begin();
//	ofClear(0,255);
//	ofEnableBlendMode(OF_BLENDMODE_ADD);
//	fboPost.getTexture().draw(0,0);
//	fboPostLines.getTexture().draw(0,0);
//	fboComposite.end();

//	ofEnableBlendMode(OF_BLENDMODE_ADD);
//	fboLines.draw(0,0,ofGetWidth(),h);
//	fboPost.draw(0,0,ofGetWidth(),h);

	fboComposite.begin();
	ofClear(0,255);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	fboLines.draw(0,0);
	fboPost.draw(0,0);
	fboComposite.end();

	fboComposite.draw(0,0,ofGetWidth(),h);

	if (m_bExportFrames || m_bRecordVideo)
	{
		recorder.save(fboComposite.getTexture());
		if(m_timeline.getCurrentTime() >= m_timeline.getOutTimeInSeconds() ||
		   m_timeline.getCurrentTime() >= m_timeline.getDurationInSeconds() ||
		   m_timeline.getCurrentFrame() >= m_timeline.getOutFrame() ||
		   m_timeline.getCurrentFrame() >= m_timeline.getDurationInFrames() ||
		   eagleFrame == 150
		   )
		{
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
