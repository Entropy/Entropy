#include "ofApp.h"

#include "Helpers.h"

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofSetDataPathRoot("../../../Resources/data/entropy/scene/Inflation");
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
	ofBackground(ofColor::black);

	// Marching Cubes
	gpuMarchingCubes.setup(300 * 1024 * 1024);

	// Noise Field
	noiseField.setup(gpuMarchingCubes.resolution);

	gpuMarchingCubes.resolution = 128;

	// Setup renderers.
	this->renderer.setup(1);
	this->renderer.resize(ofGetWidth(), ofGetHeight());


	// Setup big bang.
	for(size_t i=0;i<parameters.postBigBangColors.size();i++){
		parameters.postBigBangColors[i] = noiseField.octaves[i].color;
	}

	transitionParticles.setup();
	clearParticlesVel.loadCompute("shaders/compute_clear_color.glsl");

	cameraDistanceBeforeBB = 1;
	this->camera.setDistance(cameraDistanceBeforeBB);
	this->camera.setNearClip(0.01f);
	this->camera.setFarClip(6.0f);

	now = 0;
	t_bigbang = 0;
	parameters.state = PreBigBang;
	noiseField.scale = 1;
	for(size_t i=0;i<noiseField.octaves.size()/2;i++){
		noiseField.octaves[i].color = parameters.preBigbangColors[i];
	}
	resetWavelengths();
	needsParticlesUpdate = true;

	ofFbo::Settings settings;
	settings.width = entropy::GetSceneWidth();
	settings.height = entropy::GetSceneHeight();
	settings.internalformat = GL_RGBA32F;
	settings.numSamples = 4;

	fbo.allocate(settings);
	fboCooldown.allocate(settings);
	postFbo.allocate(settings);
	postFboCooldown.allocate(settings);
	postEffects.resize(fbo.getWidth(), fbo.getHeight());

	parameters.render.record.setSerializable(false);
	parameters.render.recordVideo.setSerializable(false);
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("Fira Code", 11, true, true, 72);
	gui.setup("Inflation","settings.json");
	gui.add(noiseField.parameters);
	gui.add(gpuMarchingCubes.parameters);
	gui.add(renderer.parameters);
	gui.add(postParameters);
	coolDownParameters.setName("Cooldown Renderer");
	coolDownParameters.add(parameters.enableCooldown);
	coolDownParameters.add(parameters.showCooldown);
	gui.add(coolDownParameters);
	gui.add(parameters);


	gui.minimizeAll();

	cameraPath.arc(glm::vec3(0), 0.6, 0.5, 0, 360, true, 4800);
	for(auto & p: cameraPath){
		std::swap(p.y, p.z);
	}

	timeline.setup();
	timeline.setDefaultFontPath("Fira Code");
	timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
	timeline.setDurationInSeconds(60*10);
	timeline.setAutosave(false);
	timeline.addFlags("subscene labels");

	gui.setTimeline(&timeline);

	listeners.push_back(ofEvents().mousePressed.newListener(this, &ofApp::mousePressed));

	listeners.push_back(timeline.events().pageChanged.newListener([this](ofxTLPageEventArgs&page){
		timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
	}));


	listeners.push_back(parameters.render.record.newListener([this](bool & record){
		if(record){
			auto path = ofSystemLoadDialog("Record to folder:",true);
			if(path.bSuccess){
				recorderPath = path.getPath();
				ofxTextureRecorder::Settings recorderSettings(fbo.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path.getPath();
				recorder.setup(recorderSettings);
				if(parameters.enableCooldown){
					auto path = recorderPath + "_cooleddown";
					recorderSettings.folderPath = path;
					recorderCooldown.setup(recorderSettings);
				}
			}else{
				this->parameters.render.record = false;
			}
		}else{
			recorder.stop();
			if(parameters.enableCooldown){
				recorderCooldown.stop();
			}
		}
	}));


	listeners.push_back(parameters.render.recordVideo.newListener([this](bool & record){
		if(record){
			auto path = ofSystemSaveDialog("video.mp4", "Record to video:");
			if(path.bSuccess){
				videoRecorderPath = path.getPath();
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), parameters.render.fps);
				recorderSettings.videoPath = videoRecorderPath;
//				recorderSettings.videoCodec = "libx264";
//				recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				recorderSettings.videoCodec = "prores";
				recorderSettings.extrasettings = "-profile:v 0";
				recorder.setup(recorderSettings);

				if(parameters.enableCooldown){
					auto path = ofFilePath::join(
								ofFilePath::getEnclosingDirectory(videoRecorderPath),
								ofFilePath::getBaseName(videoRecorderPath) + "_cooleddown.mov");
					recorderSettings.videoPath = path;
					recorderCooldown.setup(recorderSettings);
				}
			}else{
				this->parameters.render.recordVideo = false;
			}
		}else{
			recorder.stop();
			if(parameters.enableCooldown){
				recorderCooldown.stop();
			}
		}
	}));

	listeners.push_back(parameters.enableCooldown.newListener([this](bool&enabled){
		if(parameters.render.recordVideo){
			if(enabled){
				auto path = ofFilePath::join(
							ofFilePath::getEnclosingDirectory(videoRecorderPath),
							ofFilePath::getBaseName(videoRecorderPath) + "_cooleddown.mov");
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), parameters.render.fps);
				recorderSettings.videoPath = path;
	//				recorderSettings.videoCodec = "libx264";
	//				recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				recorderSettings.videoCodec = "prores";
				recorderSettings.extrasettings = "-profile:v 0";
				recorderCooldown.setup(recorderSettings);
			}else{
				recorderCooldown.stop();
			}
		}
		if(parameters.render.record){
			if(enabled){
				auto path = recorderPath + "_cooleddown";
				ofxTextureRecorder::Settings recorderSettings(fbo.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path;
				recorder.setup(recorderSettings);
				recorderCooldown.setup(recorderSettings);
			}else{
				recorderCooldown.stop();
			}
		}
	}));

	listeners.push_back(parameters.render.fps.newListener([this](int & fps){
		if(parameters.render.systemClock){
			ofSetTimeModeSystem();
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(fps));
		}
	}));

	listeners.push_back(parameters.render.systemClock.newListener([this](bool & systemClock){
		if(systemClock){
			ofSetTimeModeSystem();
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(parameters.render.fps));
		}
	}));
	
	listeners.push_back(gui.savePressedE.newListener([this]{
		auto saveTo = ofSystemSaveDialog(ofGetTimestampString() + ".json", "save settings");
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
				ofSerialize(xml, gui.getParameter());
				timeline.saveTracksToFolder(timelineDir);
				timeline.saveStructure(timelineDir);
				xml.save(path);
			}else if(extension == ".json"){
				ofJson json = ofLoadJson(path);
				ofSerialize(json, gui.getParameter());
				timeline.saveTracksToFolder(timelineDir);
				timeline.saveStructure(timelineDir);
				ofSavePrettyJson(path, json);
			}else{
				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
			}
		}
		return true;
	}));


	listeners.push_back(gui.loadPressedE.newListener([this]{
		auto loadFrom = ofSystemLoadDialog("load settings");
		if(loadFrom.bSuccess){
			auto path = std::filesystem::path(loadFrom.getPath());
			auto folder = path.parent_path();
			auto basename = path.stem().filename().string();
			auto extension = ofToLower(path.extension().string());
			auto timelineDir = (folder / (basename + "_timeline")).string();
			if(extension == ".xml"){
				ofXml xml;
				xml.load(path);
				ofDeserialize(xml, gui.getParameter());
				timeline.loadStructure(timelineDir);
				timeline.loadTracksFromFolder(timelineDir);
				timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
				gui.refreshTimelined(&timeline);
			}else
			if(extension == ".json"){
				ofJson json;
				ofFile jsonFile(path);
				jsonFile >> json;
				ofDeserialize(json, gui.getParameter());
				timeline.loadStructure(timelineDir);
				timeline.loadTracksFromFolder(timelineDir);
				timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
				gui.refreshTimelined(&timeline);
			}else{
				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
			}
			resetWavelengths();
			forceRedraw = true;
		}
		return true;
	}));

	auto createCircle = [this](float&){
		if(parameters.circle.pct>0.0001 && parameters.circle.pct<1){
			ofPolyline poly;
			auto r = (parameters.circle.inner + parameters.circle.outer) / 2.;
			auto thickness = parameters.circle.outer - parameters.circle.inner;
			poly.arc(glm::vec3(0), r, r, 0, 360 * parameters.circle.pct, true, 200);
			entropy::thickline(poly.getVertices(), thickness, circle);
		}else if(parameters.circle.pct>0.0001){
			ofPath path;
			path.setCircleResolution(200);
			auto r = (parameters.circle.inner + parameters.circle.outer) / 2.;
			auto thickness = parameters.circle.outer - parameters.circle.inner;
			path.arc(glm::vec3(0), r + thickness/2., r + thickness/2., 0, 360 * parameters.circle.pct);
			path.arc(glm::vec3(0), r - thickness/2., r - thickness/2., 0, 360 * parameters.circle.pct);
			circle = path.getTessellation();
		}else{
			circle.clear();
		}
	};
	listeners.push_back(parameters.circle.inner.newListener(createCircle));
	listeners.push_back(parameters.circle.outer.newListener(createCircle));
	listeners.push_back(parameters.circle.pct.newListener(createCircle));

	listeners.push_back(parameters.equations.newCluster.newListener([this]{
		int state = parameters.state;
		if(state == BigBang){
			clusters.emplace_back();
			auto & cluster = clusters.back();
			auto now = timeline.getCurrentTime();
			//cluster.origin = glm::vec3(ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25));
			cluster.origin = glm::vec3(ofSignedNoise(now) * 0.25, ofSignedNoise(now * glm::pi<float>()) * 0.25, ofSignedNoise(now * 2.71828) * 0.25);
			cluster.scale = 0;//parameters.rotationRadius * 0.6;
			cluster.startTime = now;
			cluster.startScale = cluster.scale;
			cluster.negativeSpace = false;
			cluster.alpha = 0;
			cluster.rotation = glm::angleAxis(ofRandom(glm::two_pi<float>()), glm::normalize(glm::vec3(ofRandom(1),ofRandom(1),ofRandom(1))));
		}
	}));

//	listeners.push_back(parameters.equations.newNegativeCluster.newListener([this]{
//		int state = parameters.state;
//		if(state == BigBang){
//			clusters.emplace_back();
//			auto & cluster = clusters.back();
//			cluster.origin = glm::vec3(ofSignedNoise(now) * 0.25, ofSignedNoise(now * glm::pi<float>()) * 0.25, ofSignedNoise(now * 2.71828) * 0.25);
//			cluster.scale = parameters.rotationRadius * 0.6;
//			cluster.startTime = now;
//			cluster.startScale = cluster.scale;
//			cluster.negativeSpace = true;
//			cluster.alpha = 0;
//			cluster.rotation = glm::angleAxis(ofRandom(glm::two_pi<float>()), glm::normalize(glm::vec3(ofRandom(1),ofRandom(1),ofRandom(1))));
//		}
//	}));

	listeners.push_back(parameters.offsetTimeline.newListener([this]{
		auto answer = ofSystemTextBoxDialog("offset in ms","");
		auto offset_ms = ofToInt(answer);
		for(auto * page: timeline.getPages()){
			for(auto * track: page->getTracks()){
				auto * keyframes = static_cast<ofxTLKeyframes*>(track);
				for(auto * kf: keyframes->getKeyframes()){
					kf->time = ofClamp(kf->time + offset_ms, 0, timeline.getDurationInMilliseconds());
				}
				keyframes->recomputePreviews();
			}
		}
	}));

	listeners.push_back(parameters.offsetSelected.newListener([this]{
		auto answer = ofSystemTextBoxDialog("offset in ms","");
		auto offset_ms = ofToInt(answer);
		for(auto * page: timeline.getPages()){
			for(auto * track: page->getTracks()){
				auto * keyframes = static_cast<ofxTLKeyframes*>(track);
				for(auto * kf: keyframes->getSelectedKeyframes()){
					kf->time = ofClamp(kf->time + offset_ms, 0, timeline.getDurationInMilliseconds());
				}
				keyframes->recomputePreviews();
			}
		}
	}));

	listeners.push_back(gui.getParameter().castGroup().parameterChangedE().newListener([this](ofAbstractParameter&p){
		forceRedraw = true;
	}));

	listeners.push_back(parameters.colors.color1.newListener([this](ofFloatColor & color){
		noiseField.octaves[0].color = color;
	}));

	listeners.push_back(parameters.colors.color2.newListener([this](ofFloatColor & color){
		noiseField.octaves[1].color = color;
	}));

	listeners.push_back(parameters.colors.color3.newListener([this](ofFloatColor & color){
		noiseField.octaves[2].color = color;
	}));

	listeners.push_back(parameters.colors.color4.newListener([this](ofFloatColor & color){
		noiseField.octaves[3].color = color;
	}));
}

//--------------------------------------------------------------
void ofApp::resetWavelengths()
{
	float wl = noiseField.resolution/4;
	targetWavelengths[0] = wl;
	noiseField.octaves[0].wavelength = wl;
	noiseField.octaves[0].advanceTime = true;
	noiseField.octaves[0].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[1] = wl;
	noiseField.octaves[1].wavelength = wl;
	noiseField.octaves[1].advanceTime = true;
	noiseField.octaves[1].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[2] = wl;
	noiseField.octaves[2].wavelength = wl;
	noiseField.octaves[2].advanceTime = true;
	noiseField.octaves[2].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[3] = wl;
	noiseField.octaves[3].wavelength = wl;
	noiseField.octaves[3].advanceTime = true;
	noiseField.octaves[3].frequencyTime = 1/wl;
	for(size_t i=4;i<noiseField.octaves.size();i++){
		noiseField.octaves[i].enabled = false;
	}
}

void ofApp::resetWavelength(size_t octave){
	auto wl = noiseField.resolution/4;
	for(size_t i=0;i<octave;i++){
		wl /= 2;
	}
	noiseField.octaves[octave].wavelength = wl / clusters[0].scale;
	noiseField.octaves[octave].advanceTime = true;
}

//--------------------------------------------------------------
void ofApp::exit()
{

}

//--------------------------------------------------------------
void ofApp::update(){
	dt = timeline.getCurrentTime() - now;
	now = timeline.getCurrentTime();
	if ((parameters.runSimulation && abs(dt) > 0.001) || forceRedraw) {
		int state = parameters.state;
		switch(state){
			case PreBigBang:
				if(state != prevState){
					resetWavelengths();
					clusters.resize(1);
					clusters[0].scale = 1;
					clusters[0].origin = glm::vec3();
					noiseField.scale = 1;
				}
//				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
//					noiseField.octaves[i].color = parameters.preBigbangColors[i];
//				}
			break;
//			case PreBigBangWobble:{
//				t_from_bigbang = now - t_bigbang;
//				float pct = t_from_bigbang/parameters.equations.preBigBangWobbleDuration;
//				pct *= pct;
//				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
//					noiseField.octaves[i].wavelength = targetWavelengths[i] * ofMap(pct, 0, 1, 1, 0.4);
//					noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * (1+pct);
////					auto color = parameters.preBigbangColors[i];
////					noiseField.octaves[i].color = color.lerp(parameters.postBigBangColors[i], pct * 0.5);
//				}
//			}break;

			case BigBang:{
				//cout << "bigbang" << endl;
				if(state != prevState){
					t_bigbang = now;
					clusters[0].startTime = now;
					clusters[0].startScale = clusters[0].scale;
				}
				t_from_bigbang = now - t_bigbang;

				for(auto & cluster: clusters){
					ofParameter<float> logScale{"", cluster.startScale,cluster.startScale, parameters.equations.maxClusterScale, ofParameterScale::Logarithmic};
					logScale.setPctScaled(ofMap(now - cluster.startTime, 0, 20 / parameters.equations.Ht, 0, 1));
					auto currentScale = logScale.get();
					logScale.setPctScaled(ofMap(now - dt - cluster.startTime, 0, 20 / parameters.equations.Ht, 0, 1));
					auto prevScale = logScale.get();
					cluster.scale += currentScale - prevScale;
					if(cluster.alpha<1){
						if(cluster.negativeSpace){
							cluster.alpha = ofxeasing::map_clamp(cluster.scale, 2, 5, 0, 1, ofxeasing::sine::easeIn);
						}else{
							cluster.alpha = ofxeasing::map_clamp(cluster.scale, parameters.colors.alphaScaleStart, parameters.colors.alphaScaleEnd, 0, 1, ofxeasing::sine::easeIn);
						}
					}
				}
				auto negative = std::count_if(clusters.begin(), clusters.end(),
								[](const Cluster & cluster){ return cluster.negativeSpace; });
				auto positive = std::count_if(clusters.begin(), clusters.end(),
								[](const Cluster & cluster){ return !cluster.negativeSpace; });
				if(timeline.getCurrentTime()>60+41){
					if(positive>2 && negative < positive && noiseField.noiseSpeed > 10){
						auto newNegatives = positive - negative;
						for(auto i=0; i<newNegatives; i++){
							cout << "adding negative" << endl;
							clusters.emplace_back();
							auto & cluster = clusters.back();
							cluster.origin = glm::vec3(ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25));
							cluster.scale = parameters.rotationRadius * 0.6;
							cluster.startTime = now;
							cluster.startScale = cluster.scale;
							cluster.negativeSpace = true;
							cluster.negativeWire = ofRandomuf() < 0.8;
							cluster.negativeFill = ofRandom(renderer.parameters.fillAlpha, renderer.parameters.fillAlpha * 2);
							cluster.alpha = 0;
							cluster.rotation = glm::angleAxis(ofRandom(glm::two_pi<float>()), glm::normalize(glm::vec3(ofRandom(1),ofRandom(1),ofRandom(1))));
						}
					}
				}else{
					if(positive>4 && negative < positive/4 && noiseField.noiseSpeed > 10){
						auto newNegatives = positive/4 - negative;
						for(auto i=0; i<newNegatives; i++){
							cout << "adding negative" << endl;
							clusters.emplace_back();
							auto & cluster = clusters.back();
							cluster.origin = glm::vec3(ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25),ofRandom(-0.25, 0.25));
							cluster.scale = parameters.rotationRadius * 0.6;
							cluster.startTime = now;
							cluster.startScale = cluster.scale;
							cluster.negativeSpace = true;
							cluster.alpha = 0;
							cluster.rotation = glm::angleAxis(ofRandom(glm::two_pi<float>()), glm::normalize(glm::vec3(ofRandom(1),ofRandom(1),ofRandom(1))));
						}
					}
				}
				auto size = clusters.size();
				clusters.erase(
					std::remove_if(clusters.begin(), clusters.end(), [this](Cluster & cluster){ return cluster.scale > parameters.equations.maxClusterScale || cluster.startTime > now; }),
					clusters.end());
				if(clusters.size()<size){
					cout << "removed " << size - clusters.size() << endl;
				}
//				noiseField.scale = scale;
//				auto pct = t_from_bigbang/parameters.equations.bigBangDuration;
//				if (this->parameters.controlCamera)
//				{
//					camera.setDistance(ofMap(pct, 0, 1, cameraDistanceBeforeBB, 0.5));
//				}
//				if (t_from_bigbang > parameters.equations.bigBangDuration) {
//					//resetWavelengths();
//					firstCycle = true;
//					parameters.state = Expansion;
//				}
//				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
////					auto color = parameters.preBigbangColors[i];
////					noiseField.octaves[i].color = color.lerp(parameters.postBigBangColors[i], 0.5 + pct * 0.5);
//					noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * ofMap(pct,0,1,2,1);
//				}
//				noiseField.octaves.back().wavelength = targetWavelengths.back() * glm::clamp(1 - scale * 2, 0.8f, 1.f);
			}break;
			case Expansion:
				if(prevState!=state){
					t_expansion = now;
					clusters.emplace(clusters.begin());
					clusters.front().origin = glm::vec3();
					clusters.front().scale = parameters.equations.mainClusterScaleAfterBigbang;
					clusters.front().startScale = parameters.equations.mainClusterScaleAfterBigbang;
					clusters.front().negativeSpace = false;
					clusters.front().startTime = now;
				}
				t_from_expansion = now - t_expansion;
//				if(clusters[0].scale<0.8){
//					//clusters[0].scale = ofxeasing::map(t_from_expansion, 0, 1, 0.5, 1.01, ofxeasing::sine::easeOut);

//					ofParameter<float> logScale{"",clusters[0].startScale,clusters[0].startScale,30,ofParameterScale::Logarithmic};
//					logScale.setPctScaled(ofMap(now - clusters[0].startTime, 0, 20 / parameters.equations.Ht, 0, 1));
//					clusters[0].scale = logScale;
//					if(clusters[0].scale > 1){
//						clusters[0].startTime = now;
//						clusters[0].startScale = clusters[0].scale;
//					}

//				}else{
//					ofParameter<float> logScale{"",clusters[0].startScale,clusters[0].startScale,30,ofParameterScale::Logarithmic};
//					logScale.setPctScaled(ofMap(now - clusters[0].startTime, 0, 1500, 0, 1));
//					clusters[0].scale = logScale;
//				}
				clusters[0].scale = parameters.equations.mainClusterScaleAfterBigbang;
				for(auto i=1;i<clusters.size();i++){
					//clusters[i].scale = ofxeasing::map(now - clusters[i].startTime, 0, 1.5, clusters[i].startScale, 30, ofxeasing::sine::easeIn);
					//clusters[i].scale += log(clusters[i].scale+1.1) * dt * parameters.equations.Ht;

					ofParameter<float> logScale{"",clusters[i].startScale,clusters[i].startScale, parameters.equations.maxClusterScale, ofParameterScale::Logarithmic};
					logScale.setPctScaled(ofMap(now - clusters[i].startTime, 0, 20 / parameters.equations.Ht, 0, 1));
					auto currentScale = logScale.get();
					logScale.setPctScaled(ofMap(now - dt - clusters[i].startTime, 0, 20 / parameters.equations.Ht, 0, 1));
					auto prevScale = logScale.get();
					clusters[i].scale += currentScale - prevScale;
				}
				clusters.erase(
					std::remove_if(clusters.begin() + 1, clusters.end(), [this](Cluster & cluster){ return cluster.scale > parameters.equations.maxClusterScale; }),
					clusters.end());
			break;

//			case Expansion:
//				t_from_bigbang = now - t_bigbang;
//				scale += dt * parameters.equations.Ht;// t_from_bigbang/parameters.bigBangDuration;
//				noiseField.scale = scale;
//				if (this->parameters.controlCamera)
//				{
//					if (camera.getDistance() > 0.5f)
//					{
//						auto d = camera.getDistanceint(parameters.state)();
//						d -= dt * parameters.equations.Ht;
//						camera.setDistance(d);
//					}
//				}
//				if(!firstCycle){
//					for(size_t i=0;i<noiseField.octaves.size()/2;i++){
//						if(noiseField.octaves[i].advanceTime){
//							noiseField.octaves[i].wavelength -= dt * parameters.equations.Ht;
//							if(noiseField.octaves[i].wavelength < parameters.equations.hubbleWavelength){
//								noiseField.octaves[i].advanceTime = false;
//								//resetWavelength(i);
//							}
//						}
//					}
//				}
//			break;
//			case ExpansionTransition:{
//				float t_EndIn = t_transition + parameters.equations.bbTransitionIn;
//				float t_EndPlateau = t_EndIn + parameters.equations.bbTransitionPlateau;
//				float t_EndOut = t_EndPlateau + parameters.equations.bbTransitionOut;

//				if(now < t_EndIn){
//					auto pct = ofMap(now,t_transition,t_EndIn,0,1);
//					pct = sqrt(sqrt(sqrt(pct)));
//					if(firstCycle){
//						parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtBB * 2);
//					}else{
//						parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtPostBB, parameters.equations.HtBB);
//					}
//					scale  += dt * parameters.equations.Ht;
//				}else if(now < t_EndPlateau){
//					auto pct = ofMap(now,t_transition,t_EndOut,0,1);
//					pct = sqrt(sqrt(sqrt(pct)));
//					parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtPostBB);
//					scale  += dt * parameters.equations.Ht;
//				}else if(now < t_EndOut){
//					auto pct = ofMap(now,t_transition,t_EndOut,0,1);
//					pct = sqrt(sqrt(sqrt(pct)));
//					parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtPostBB);
//					scale  += dt * parameters.equations.Ht;
//				}
//				noiseField.scale = scale;
//			}break;

			case ParticlesTransition:{
				if(prevState!=state){
					cout << "entering parent particles" << endl;
					t_from_particles = now;
					needsParticlesUpdate = true;
					auto vertices = this->gpuMarchingCubes.getNumVertices();
					auto size = vertices * sizeof(glm::vec4) * 2;
					transitionParticlesPosition.allocate(size, GL_STATIC_DRAW);
					this->gpuMarchingCubes.getGeometry().getVertexBuffer().copyTo(transitionParticlesPosition,0,0,size);
					clearParticlesVel.begin();
					transitionParticlesPosition.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
					clearParticlesVel.dispatchCompute(vertices / 1024 + 1, 1, 1);
					clearParticlesVel.end();

					this->transitionParticles.setTotalVertices(vertices);
				}
				float alphaParticles = (now - t_from_particles) / parameters.equations.transitionParticlesDuration;
				alphaParticles = glm::clamp(alphaParticles, 0.f, 1.f);
				transitionParticles.color = ofFloatColor(transitionParticles.color, alphaParticles);

				float alphaBlobs = (now - t_from_particles) / parameters.equations.transitionBlobsOutDuration;
				alphaBlobs = 1.0f - glm::clamp(alphaBlobs, 0.f, 1.f);
				//noiseField.speedFactor = alphaBlobs;
				renderer.parameters.alphaFactor = alphaBlobs;
				this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
			}break;
		}
		noiseField.update(dt);
		if (renderer.parameters.alphaFactor > 0.001) {
			gpuMarchingCubes.update(noiseField.getTexture());
		}

		if (needsParticlesUpdate) {

			this->transitionParticles.setTotalVertices(this->gpuMarchingCubes.getNumVertices());
			this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
			needsParticlesUpdate = false;
		}
		prevState = state;

//		auto * colors = gpuMarchingCubes.getGeometry().getColorBuffer().map<ofFloatColor>(GL_WRITE_ONLY);
//		for(auto i=0;i<20;i++){
//			colors[(ofGetFrameNum() + i) % gpuMarchingCubes.getNumVertices()] = ofFloatColor(3);
//		}
//		gpuMarchingCubes.getGeometry().getColorBuffer().unmap();

		//auto distance = this->getCamera(render::Layout::Back)->getEasyCam().getDistance();
		//this->getCamera(render::Layout::Back)->getEasyCam().orbitDeg(ofGetElapsedTimef()*10.f,0,distance,glm::vec3(0,0,0));

		//transitionParticles.color = ofFloatColor(transitionParticles.color, 0.0);
	}

//	if(save){
//		static size_t idx = 0;
//		auto mesh = gpuMarchingCubes.downloadGeometry();
//		mesh.save(ofToString(idx,0,4)+".ply", true);
//		idx += 1;
//	}

	if((parameters.rotating && abs(dt) > 0.001) || forceRedraw){
		if(parameters.rotatingUseSpeed){
			parameters.rotationRadius -= dt * parameters.rotationRadiusSpeed;
		}
		camera.orbitDeg(orbitAngle, 0, parameters.rotationRadius);
		orbitAngle += dt * parameters.rotationSpeed;
		orbitAngle = ofWrap(orbitAngle, 0, 360);

		/*camera.setPosition(cameraPath[ofGetFrameNum() % cameraPath.size()] * parameters.rotationRadius.get());
		camera.lookAt(glm::vec3(0), glm::vec3(0,1,0));*/
	}
	
	if(dofTimeStart > 0){
		auto diff = std::chrono::duration<double>(now - dofTimeStart).count();
		auto d = ofxeasing::map_clamp(diff, 0, 1, dofDistanceStart, dofDistanceTarget, ofxeasing::back::easeOut);
		renderer.parameters.dofDistance = d;
		if(diff>=1){
			dofTimeStart = 0;
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	if((parameters.runSimulation && abs(dt) > 0.001) || forceRedraw){
		auto drawNegativeSpace = [this]{
			camera.begin();
			cout << "negative space!" << endl;
			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
			std::vector<ofFloatColor> colors;
			colors.push_back(noiseField.octaves[0].color);
			colors.push_back(noiseField.octaves[1].color);
			colors.push_back(noiseField.octaves[2].color);
			colors.push_back(noiseField.octaves[3].color);
			float alphaFactor = renderer.parameters.alphaFactor;
			bool wire = renderer.parameters.wireframe;
			float fillAlpha = renderer.parameters.fillAlpha;

			noiseField.octaves[0].color = parameters.negativeColors.color1;
			noiseField.octaves[1].color = parameters.negativeColors.color2;
			noiseField.octaves[2].color = parameters.negativeColors.color3;
			noiseField.octaves[3].color = parameters.negativeColors.color4;
			noiseField.update(0);
			if (renderer.parameters.alphaFactor > 0.001) {
				gpuMarchingCubes.update(noiseField.getTexture());
			}

			if (needsParticlesUpdate) {
				this->transitionParticles.setTotalVertices(this->gpuMarchingCubes.getNumVertices());
				this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
				needsParticlesUpdate = false;
			}
			auto first = true;
			renderer.parameters.wireframeColor = true;
			for(auto & cluster: clusters){
		//		glm::mat4 model = glm::translate(cluster.origin);
		//		model = glm::scale(model, glm::vec3(cluster.scale));
				if(cluster.negativeSpace){
					ofNode node;
					node.setScale(cluster.scale);
					node.setPosition(cluster.origin);
					if(!first){
						//node.lookAt(camera.getPosition(), glm::vec3(0,1,0));
						node.setOrientation(cluster.rotation);
					}
					const auto & model = node.getLocalTransformMatrix();
					if(!cluster.negativeWire){
						renderer.parameters.wireframe = false;
						renderer.parameters.fillAlpha = ofMap(cluster.alpha, 0, 0.8, 0, 1);
					}else{
						renderer.parameters.alphaFactor = cluster.alpha;
					}
					renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), GL_TRIANGLES, camera, model);
				}
				first = false;
			}
			renderer.parameters.wireframeColor = false;
			noiseField.octaves[0].color = colors[0];
			noiseField.octaves[1].color = colors[1];
			noiseField.octaves[2].color = colors[2];
			noiseField.octaves[3].color = colors[3];
			renderer.parameters.alphaFactor = alphaFactor;
			renderer.parameters.wireframe = wire;
			renderer.parameters.fillAlpha = fillAlpha;
			camera.end();
		};


		auto hasNegativeSpace = false;
		for(auto & cluster: clusters){
			hasNegativeSpace |= cluster.negativeSpace;
		}

		fbo.begin();
		ofClear(0,255);
		camera.begin();
		int state = parameters.state;
		switch (state) {
			case PreBigBang:
			case BigBang:
			case Expansion:
			case ExpansionTransition:{
				ofDisableAlphaBlending();
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);

				auto first = true;
				float alphaFactor = renderer.parameters.alphaFactor;
				bool wire = renderer.parameters.wireframe;
				float fillAlpha = renderer.parameters.fillAlpha;
				for(auto & cluster: clusters){
					if(!cluster.negativeSpace){
						ofNode node;
						node.setScale(cluster.scale);
						node.setPosition(cluster.origin);
						if(!first){
							node.setOrientation(cluster.rotation);
						}
						renderer.parameters.alphaFactor = cluster.alpha;
						const auto & model = node.getLocalTransformMatrix();
						renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), GL_TRIANGLES, camera, model);

					}
					first = false;
				}
				renderer.parameters.alphaFactor = alphaFactor;
				renderer.parameters.wireframe = wire;
				renderer.parameters.fillAlpha = fillAlpha;
			}break;
			case ParticlesTransition:{
				//renderer.clip = false;
				auto & cluster = clusters[0];
				ofNode node;
				node.setScale(cluster.scale);
				node.setPosition(cluster.origin);
				const auto & model = node.getLocalTransformMatrix();
				if (renderer.parameters.alphaFactor > 0.001) {
					ofDisableAlphaBlending();
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_ONE);
					renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), GL_TRIANGLES, camera, model);
				}
				ofDisableAlphaBlending();
				glEnable(GL_BLEND);
				glBlendFunc(GL_ONE, GL_ONE);
				//ofEnableBlendMode(OF_BLENDMODE_ALPHA);
				auto alphaBlobs = renderer.parameters.alphaFactor;
				renderer.parameters.alphaFactor = 1;
				renderer.draw(transitionParticles.getVbo(), 0, transitionParticles.getNumVertices(), GL_TRIANGLES, camera, model);
				renderer.parameters.alphaFactor = alphaBlobs;
				break;
			}break;
		}


	//	switch (state) {
	//	case PreBigBang:
	//	case PreBigBangWobble:
	//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//		renderer.clip = true;
	//		renderer.fadeEdge0 = 0.0;
	//		renderer.fadeEdge1 = 0.5;
	//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
	//		break;
	//	case BigBang:
	//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//		renderer.fadeEdge0 = 0.0;
	//		renderer.fadeEdge1 = 0.5;
	//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);

	//		ofEnableBlendMode(OF_BLENDMODE_ADD);
	//		renderer.fadeEdge0 = scale*scale;
	//		renderer.fadeEdge1 = scale;
	//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
	//		break;
	//	case Expansion:
	//	case ExpansionTransition:
	//		ofEnableBlendMode(OF_BLENDMODE_ADD);
	//		renderer.clip = false;
	//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
	//		break;
	//	case ParticlesTransition:
	//		renderer.clip = false;
	//		if (renderer.alphaFactor > 0.001) {
	//			ofEnableBlendMode(OF_BLENDMODE_ADD);
	//			renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
	//		}
	//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	//		auto alphaBlobs = renderer.alphaFactor;
	//		renderer.alphaFactor = 1;
	//		renderer.draw(transitionParticles.getVbo(), 0, transitionParticles.getNumVertices(), camera);
	//		renderer.alphaFactor = alphaBlobs;
	//		break;
	//	}

		//ofDrawSphere(dofTarget, 0.01);

		camera.end();

		if(renderer.parameters.enableDOF && renderer.parameters.dofSamples > 1 && hasNegativeSpace){
			drawNegativeSpace();
		}
		fbo.end();

		postEffects.process(fbo.getTexture(), postFbo, postParameters);

		postFbo.begin();

		if((!renderer.parameters.enableDOF || renderer.parameters.dofSamples == 1) && hasNegativeSpace){
			drawNegativeSpace();
		}


		ofCamera camera;
		auto fov = camera.getFov();
		float eyeY = 1;
		float halfFov = PI * fov / 360.;
		float theTan = tanf(halfFov);
		float d = eyeY / theTan;
		camera.setPosition({0,0,d});
		camera.lookAt({0,0,0}, {0,1,0});
		camera.setNearClip(0.01);
		camera.setFarClip(10);
		ofSetColor(255);
		camera.begin();
		circle.draw();
		camera.end();

		postFbo.end();

		if(parameters.render.record || parameters.render.recordVideo){
			recorder.save(postFbo.getTexture());
		}
	}

	auto w = ofGetWidth();
	auto h = w * postFbo.getHeight() / postFbo.getWidth();

	if(!parameters.showCooldown){
		ofDisableAlphaBlending();
		ofSetColor(255);
		postFbo.draw(0,0,w,h);
	}

	if((parameters.runSimulation && abs(dt) > 0.001) || forceRedraw){

		if(parameters.enableCooldown || parameters.showCooldown){
			fboCooldown.begin();
			cout << "redrawing as cooldowned " << endl;
			ofClear(0,255);
			camera.begin();
			ofDisableAlphaBlending();
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			//gpuMarchingCubes.getGeometry().draw(GL_TRIANGLES, 0, gpuMarchingCubes.getNumVertices());
			auto first = true;
			for(auto & cluster: clusters){
				if(!cluster.negativeSpace){
					ofNode node;
					node.setScale(cluster.scale);
					node.setPosition(cluster.origin);
					if(!first){
						node.setOrientation(cluster.rotation);
					}
					entropy::render::WireframeFillRenderer::DrawSettings drawSettings;
					drawSettings.camera = &camera;
					drawSettings.mode = GL_TRIANGLES;
					drawSettings.model = node.getLocalTransformMatrix();
					drawSettings.numVertices = gpuMarchingCubes.getNumVertices();
					drawSettings.offset = 0;
					drawSettings.parameters = coolDownParameters;
					renderer.draw(gpuMarchingCubes.getGeometry(), drawSettings);

				}
				first = false;
			}

			camera.end();
			fboCooldown.end();

			postEffects.process(fboCooldown.getTexture(), postFboCooldown, postParameters);

			if(parameters.render.record || parameters.render.recordVideo){
				recorderCooldown.save(postFboCooldown.getTexture());
			}
		}
	}

	forceRedraw = false;


	if(parameters.showCooldown){
		ofDisableAlphaBlending();
		ofSetColor(255);
		postFboCooldown.draw(0,0,w,h);
	}

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	if(showTimeline){
		ofSetColor(0);
		ofDrawRectangle(timeline.getTopLeft(), timeline.getWidth(), timeline.getHeight());
		ofSetColor(255);
		timeline.draw();
	}
	gui.draw();
	ofDrawBitmapString(ofGetFrameRate(), ofGetWidth()-100, 20);

	ofSetColor(0,200);
	ofDrawRectangle(ofGetWidth() - 230, 30, 220, 220);

	ofPushMatrix();
	ofTranslate(ofGetWidth() - 120, 140);
	ofScale(100,100,0);

	ofSetColor(255);

	renderer.getBokehShape().draw();
	ofPopMatrix();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 't':
			showTimeline = !showTimeline;
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
void ofApp::mousePressed(ofMouseEventArgs & mouse){
	if(mouse.button==OF_MOUSE_BUTTON_LEFT && mouse.hasModifier(OF_KEY_CONTROL) && mouse.hasModifier(OF_KEY_SHIFT)){
		auto geometry = gpuMarchingCubes.downloadGeometry();
		auto v = camera.worldToScreen(geometry.getVertex(0));
		auto nearest = glm::distance2(v.xy(), mouse.xy());
		for(auto & v: geometry.getVertices()){
			auto screenv = camera.worldToScreen(v);
			auto d = glm::distance2(screenv.xy(), mouse.xy());
			if(d<nearest){
				nearest = d;
				dofTarget = v;
			}
		}
		renderer.parameters.dofDistance = glm::distance(camera.getPosition(), dofTarget);
//		dofTimeStart = timeline.getCurrentTime();
//		dofDistanceStart = renderer.dofDistance;
//		dofDistanceTarget = glm::distance(camera.getPosition(), dofTarget);
	}
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
