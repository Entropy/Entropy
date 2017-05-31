#include "ofApp.h"
#include "ofxSerialize.h"
#include "Helpers.h"
#include "Octree.h"

//--------------------------------------------------------------
const string ofApp::kSceneName = "Particles";
const float ofApp::kHalfDim = 400.0f;
const unsigned int ofApp::kMaxLights = 16u;

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofSetDataPathRoot(entropy::GetSceneDataPath(kSceneName).string());
	//ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
	ofBackground(ofColor::black);	
	
	// Initialize particle system.
	environment = nm::Environment::Ptr(new nm::Environment(glm::vec3(-kHalfDim), glm::vec3(kHalfDim)));
	particleSystem.init(environment);
	photons.init(environment);
	pointLights.resize(kMaxLights);

	// Initialize transform feedback.
	feedbackBuffer.allocate(1024 * 1024 * 300, GL_STATIC_DRAW);
	auto stride = sizeof(glm::vec4) * 3;// + sizeof(glm::vec3);
	feedbackVbo.setVertexBuffer(feedbackBuffer, 4, stride, 0);
	feedbackVbo.setColorBuffer(feedbackBuffer, stride, sizeof(glm::vec4));
	feedbackVbo.setNormalBuffer(feedbackBuffer, stride, sizeof(glm::vec4) * 2);
	glGenQueries(1, &numPrimitivesQuery);

	// Init fbos
	ofFbo::Settings fboSettings;
	fboSettings.width = entropy::GetSceneWidth();
	fboSettings.height = entropy::GetSceneHeight();
	fboSettings.internalformat = GL_RGBA32F;
	fboSettings.useDepth = true;
	fboSettings.useStencil = false;
	fboSettings.numSamples = 4;

	fboSceneFront.allocate(fboSettings);
	fboSceneBack.allocate(fboSettings);
	fboPostFront.allocate(fboSettings);
	fboPostBack.allocate(fboSettings);

	// Init post effects
	this->postEffects.resize(fboSettings.width, fboSettings.height);
	this->renderer.resize(fboSettings.width, fboSettings.height);

	// Setup renderers.
	this->renderer.setup(kHalfDim);
	this->renderer.resize(fboSettings.width, fboSettings.height);
	this->renderer.parameters.fogMaxDistance.setMax(kHalfDim * 100);
	this->renderer.parameters.fogMinDistance.setMax(kHalfDim);

	textRendererFront.setup(kHalfDim, entropy::GetSceneWidth(), entropy::GetSceneHeight(), TextRenderer::FrontScreen);
	textRendererBack.setup(kHalfDim, entropy::GetSceneWidth(), entropy::GetSceneHeight(), TextRenderer::BackScreen);

	// Load shaders.
	shaderSettings.bindDefaults = false;
	shaderSettings.shaderFiles[GL_VERTEX_SHADER] = std::filesystem::path("shaders") / "particle.vert";
	shaderSettings.varyingsToCapture = { "out_position", "out_color", "out_normal" };
	shaderSettings.sourceDirectoryPath = std::filesystem::path("shaders");
	shaderSettings.intDefines["COLOR_PER_TYPE"] = this->parameters.rendering.colorsPerType;
	this->shader.setup(shaderSettings);

	// Setup lights.
	for (auto & light : pointLights)
	{
		light.setup();
		light.setAmbientColor(ofFloatColor::black);
		light.setSpecularColor(ofFloatColor::white);
	}

	// Add parameter listeners.
	this->eventListeners.push_back(parameters.rendering.ambientLight.newListener([&](float & ambient)
	{
		ofSetGlobalAmbientColor(ofFloatColor(ambient));
	}));
	this->eventListeners.push_back(this->parameters.rendering.colorsPerType.newListener([&](bool & colorsPerType)
	{
		shaderSettings.intDefines["COLOR_PER_TYPE"] = colorsPerType;
		this->shader.setup(shaderSettings);
	}));
	this->eventListeners.push_back(this->parameters.rendering.colorsPerType.newListener([&](bool & colorsPerType)
	{
		shaderSettings.intDefines["COLOR_PER_TYPE"] = colorsPerType;
		this->shader.setup(shaderSettings);
	}));

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("FiraCode-Light", 11, true, true, 72);
	this->gui.setup(kSceneName, "parameters.json");
	this->gui.add(this->parameters);
	this->gui.add(this->environment->parameters);
	this->gui.add(nm::Octree<nm::Particle>::PARAMETERS());
	this->gui.add(nm::Particle::parameters);
	this->gui.add(this->renderer.parameters);
	this->gui.add(this->postParams);
	this->gui.add(this->textRendererFront.parameters);
	this->gui.add(this->textRendererBack.parameters);
	this->gui.minimizeAll();
	this->eventListeners.push_back(this->gui.savePressedE.newListener([this](void)
	{
//		if (ofGetKeyPressed(OF_KEY_SHIFT))
//		{
			auto name = ofSystemTextBoxDialog("Enter a name for the preset", "");
			if (!name.empty())
			{
				return this->savePreset(name);
			}
//		}
		return true;
	}));
	this->eventListeners.push_back(this->gui.loadPressedE.newListener([this](void)
	{
//		if (ofGetKeyPressed(OF_KEY_SHIFT))
//		{
			auto result = ofSystemLoadDialog("Select a preset folder.", true, ofToDataPath("presets", true));
			if (result.bSuccess)
			{
				this->loadPreset(result.fileName);
				ofSetWindowTitle(ofFilePath::getBaseName(result.fileName));
			}
//		}
		return true;
	}));

	this->timeline.setName("timeline");
	this->timeline.setup();
	this->timeline.setDefaultFontPath("FiraCode-Light");
	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
	//this->timeline.setSpacebarTogglePlay(false);
	this->timeline.setLoopType(OF_LOOP_NONE);
	this->timeline.setFrameRate(30.0f);
	this->timeline.setDurationInSeconds(60 * 5);
	this->timeline.setAutosave(false);
	this->timeline.setPageName(this->parameters.getName());
	this->timeline.addFlags("Cues");
	this->eventListeners.push_back(this->timeline.events().viewWasResized.newListener([this](ofEventArgs &)
	{
		this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
	}));

//	const auto cameraTrackName = "Camera";
//	this->cameraTrack.setDampening(1.0f);
//	this->cameraTrack.setCamera(this->camera);
//	this->cameraTrack.setXMLFileName(this->timeline.nameToXMLName(cameraTrackName));
//	this->timeline.addTrack(cameraTrackName, &this->cameraTrack);
//	this->cameraTrack.setDisplayName(cameraTrackName);

	this->gui.setTimeline(&this->timeline);

	// Setup texture recorder.
	this->eventListeners.push_back(this->parameters.recording.recordSequence.newListener([this](bool & record)
	{
		if (record)
		{
			auto path = ofSystemLoadDialog("Record to folder:", true);
			if (path.bSuccess)
			{
				ofxTextureRecorder::Settings recorderSettingsFront(this->fboPostFront.getTexture());
				recorderSettingsFront.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettingsFront.folderPath = path.getPath() + "/front";
				ofDirectory front(recorderSettingsFront.folderPath);
				if(!front.exists()){
					front.create();
				}
				this->textureRecorderFront.setup(recorderSettingsFront);

				ofxTextureRecorder::Settings recorderSettingsBack(this->fboPostBack.getTexture());
				recorderSettingsBack.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettingsBack.folderPath = path.getPath() + "/back";
				ofDirectory back(recorderSettingsBack.folderPath);
				if(!back.exists()){
					back.create();
				}
				this->textureRecorderBack.setup(recorderSettingsBack);

				this->reset();
//				this->cameraTrack.lockCameraToTrack = true;
				this->timeline.play();
			}
			else
			{
				this->parameters.recording.recordSequence = false;
			}
		}else{
			textureRecorderFront.stop();
			textureRecorderBack.stop();
		}
	}));

	this->eventListeners.push_back(parameters.recording.recordVideo.newListener([this](bool & record)
	{
#if OFX_VIDEO_RECORDER
		if (record)
		{
			auto path = ofSystemSaveDialog("video.mp4", "Record to video:");
			if (path.bSuccess)
			{
				ofxTextureRecorder::VideoSettings recorderSettingsFront(fboPostFront.getTexture(), 60);
				recorderSettingsFront.videoPath = ofFilePath::removeExt(path.getPath()) + "_front" + ofFilePath::getFileExt(path.getPath());
				if(ofFilePath::getFileExt(path.getPath()) == ".mov"){
					recorderSettingsFront.videoCodec = "prores";
					recorderSettingsFront.extrasettings = "-profile:v 0";
				}else{
					recorderSettingsFront.videoCodec = "libx264";
					recorderSettingsFront.extrasettings = "-preset ultrafast -crf 0";
				}
				textureRecorderFront.setup(recorderSettingsFront);

				ofxTextureRecorder::VideoSettings recorderSettingsBack(fboPostBack.getTexture(), 60);
				recorderSettingsBack.videoPath = ofFilePath::removeExt(path.getPath()) + "_back" + ofFilePath::getFileExt(path.getPath());
				if(ofFilePath::getFileExt(path.getPath()) == ".mov"){
					recorderSettingsBack.videoCodec = "prores";
					recorderSettingsBack.extrasettings = "-profile:v 0";
				}else{
					recorderSettingsBack.videoCodec = "libx264";
					recorderSettingsBack.extrasettings = "-preset ultrafast -crf 0";
				}
				textureRecorderBack.setup(recorderSettingsBack);
			}
			else {
				this->parameters.recording.recordVideo = false;
			}
		}
		else {
			textureRecorderFront.stop();
			textureRecorderBack.stop();
		}
#endif
	}));

//	eventListeners.push_back(gui.savePressedE.newListener([this]{
//		auto saveTo = ofSystemSaveDialog(ofGetTimestampString() + ".json", "save settings");
//		if(saveTo.bSuccess){
//			auto path = std::filesystem::path(saveTo.getPath());
//			auto folder = path.parent_path();
//			auto basename = path.stem().filename().string();
//			auto extension = ofToLower(path.extension().string());
//			auto timelineDir = (folder / (basename + "_timeline")).string();
//			if(extension == ".xml"){
//				ofXml xml;
//				if(std::filesystem::exists(path)){
//					xml.load(path);
//				}
//				ofSerialize(xml, gui.getParameter());
//				timeline.saveTracksToFolder(timelineDir);
//				timeline.saveStructure(timelineDir);
//				xml.save(path);
//			}else if(extension == ".json"){
//				ofJson json = ofLoadJson(path);
//				ofSerialize(json, gui.getParameter());
//				timeline.saveTracksToFolder(timelineDir);
//				timeline.saveStructure(timelineDir);
//				ofSavePrettyJson(path, json);
//			}else{
//				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
//			}
//		}
//		return true;
//	}));


//	eventListeners.push_back(gui.loadPressedE.newListener([this]{
//		auto loadFrom = ofSystemLoadDialog("load settings", false, ofToDataPath("presets"));
//		if(loadFrom.bSuccess){
//			auto path = std::filesystem::path(loadFrom.getPath());
//			auto folder = path.parent_path();
//			auto basename = path.stem().filename().string();
//			auto extension = ofToLower(path.extension().string());
//			auto timelineDir = (folder / (basename + "_timeline")).string();
//			if(!ofDirectory(timelineDir).exists()){
//				timelineDir = folder;
//			}
//			if(extension == ".xml"){
//				ofXml xml;
//				xml.load(path);
//				ofDeserialize(xml, gui.getParameter());
//				timeline.loadStructure(timelineDir);
//				timeline.loadTracksFromFolder(timelineDir);
//				timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
//				gui.refreshTimelined(&timeline);
//			}else
//			if(extension == ".json"){
//				ofJson json;
//				ofFile jsonFile(path);
//				jsonFile >> json;
//				ofDeserialize(json, gui.getParameter());
//				timeline.loadStructure(timelineDir);
//				timeline.loadTracksFromFolder(timelineDir);
//				timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
//				gui.refreshTimelined(&timeline);
//			}else{
//				ofLogError("ofxGui") << extension << " not recognized, only .xml and .json supported by now";
//			}
//			reset();
//		}
//		return true;
//	}));

	eventListeners.push_back(parameters.recording.fps.newListener([this](int & fps){
		if(parameters.recording.systemClock){
			ofSetTimeModeSystem();
			ofSetVerticalSync(true);
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(fps));
			ofSetVerticalSync(false);
		}
	}));

	eventListeners.push_back(parameters.recording.systemClock.newListener([this](bool & systemClock){
		if(systemClock){
			ofSetTimeModeSystem();
			ofSetVerticalSync(true);
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(parameters.recording.fps));
			ofSetVerticalSync(false);
		}
	}));

	parameters.rendering.fov.ownListener([this](float & fov){
		camera.setFov(fov);
	});

	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	//this->loadPreset("_autosave");
	//this->reset();

	ofSetMutuallyExclusive(parameters.rendering.additiveBlending, parameters.rendering.glOneBlending);

	eventListeners.push_back(parameters.reset.resetScene.newListener(this, &ofApp::reset));

	eventListeners.push_back(environment->state.newListener([this](int & state){
		if(prevState != state && state==nm::Environment::TRANSITION_OUT){
			clusters.clear();
			clusters.emplace_back();
			clusters.front().alpha = 1;
			clusters.front().startTime = now;
		}
		prevState = (nm::Environment::State)state;
	}));

	eventListeners.push_back(parameters.rendering.addCluster.newListener([&]{
		clusters.emplace_back();
		clusters.back().origin = camera.getPosition() - camera.getLookAtDir() * kHalfDim + camera.getXAxis() * ofRandom(-kHalfDim, kHalfDim) + camera.getYAxis() * ofRandom(-kHalfDim, kHalfDim);
		clusters.back().alpha = 1;
		clusters.back().startTime = now;
		clusters.back().rotation = glm::angleAxis(ofRandom(glm::two_pi<float>()), glm::normalize(glm::vec3(ofRandom(1),ofRandom(1),ofRandom(1))));
	}));

}

//--------------------------------------------------------------
void ofApp::exit()
{
	particleSystem.clearParticles();
	
	// Clear transform feedback.
	glDeleteQueries(1, &numPrimitivesQuery);
}

//--------------------------------------------------------------
void ofApp::update()
{
	dt = timeline.getCurrentTime() - now;
	now = timeline.getCurrentTime();

	environment->update();
	photons.update(dt);
	particleSystem.update(dt);
	auto scale = environment->getExpansionScalar();

	photonsAlive.clear();
	std::copy_if(this->photons.getPosnsRef().begin(),
				 this->photons.getPosnsRef().end(),
				 std::back_inserter(photonsAlive),
				 [](nm::Photon & p){
					return p.alive && p.age < nm::Photons::LIVE();
				 });
	std::sort(photonsAlive.begin(), photonsAlive.end(), [&](nm::Photon & p1, nm::Photon & p2){
		return p1.age < p2.age;
	});

	for (auto & light : pointLights)
	{
		light.disable();
		light.setPosition(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	}


	for (size_t i = 0; i < photonsAlive.size() && i < kMaxLights; ++i)
	{
		auto & light = pointLights[i];
		light.enable();
		light.setDiffuseColor(ofFloatColor::white * parameters.rendering.lightStrength );//*  (1 - ofClamp(photonsAlive[i].age / nm::Photons::LIVE() / environment->systemSpeed, 0, 1))
		light.setPointLight();
		light.setPosition(photonsAlive[i].pos * scale);
		light.setAttenuation(0, 0, parameters.rendering.attenuation);
	}

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, numPrimitivesQuery);
	this->shader.beginTransformFeedback(GL_TRIANGLES, feedbackBuffer);
	{
		std::array<ofFloatColor, nm::Particle::NUM_TYPES> colors;
		std::transform(nm::Particle::DATA, nm::Particle::DATA + nm::Particle::NUM_TYPES, colors.begin(), [](const nm::Particle::Data & data)
		{
			return data.color.get();
		});
		this->shader.setUniform4fv("colors", reinterpret_cast<float*>(colors.data()), nm::Particle::NUM_TYPES);
		particleSystem.draw(this->shader);
	}
	this->shader.endTransformFeedback(feedbackBuffer);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(numPrimitivesQuery, GL_QUERY_RESULT, &numPrimitives);


	if(parameters.rendering.drawText){
		textRendererFront.update(particleSystem, *environment);
		textRendererBack.update(particleSystem, *environment);
	}


	// Camera position tracking next annihilation
	if(parameters.rendering.doCameraTracking){
		auto renewLookAt = arrived;
		if(arrived && lookAt.first != lookAt.second){
			auto * lookAt1 = particleSystem.getById(lookAt.first);
			auto * lookAt2 = particleSystem.getById(lookAt.second);
			if(lookAt1 && lookAt2){
				renewLookAt = glm::distance(lookAt1->pos * scale, lookAt2->pos * scale) > nm::Octree<nm::Particle>::INTERACTION_DISTANCE();
				//cout << "arrived and renew " << renewLookAt << endl;
			}
		}
		if(renewLookAt && timeConnectionLost==0){
			timeConnectionLost = now;
			lookAt.first = lookAt.second = 0;
			//cout << "renew started counter" << endl;
		}

		auto timesinceConnectionLost = now - timeConnectionLost;
		if(renewLookAt && timesinceConnectionLost > parameters.rendering.minTimeBetweenTravels){
			//cout << "trying to renew" << endl;
			auto particles = particleSystem.getParticles();
			std::vector<nm::Particle*> sortedParticles(particles.size());
			std::transform(particles.begin(),
						   particles.end(),
						   sortedParticles.begin(),
						   [](nm::Particle & p){ return &p; });

			std::sort(sortedParticles.begin(), sortedParticles.end(), [&](nm::Particle * p1, nm::Particle * p2){
				return glm::distance2(p1->pos, camera.getGlobalPosition()) > glm::distance2(p2->pos, camera.getGlobalPosition());
			});


			switch(environment->state.get()){
				case nm::Environment::BARYOGENESIS:
					for(auto * p1: sortedParticles){
						for(auto * p2: p1->potentialInteractionPartners){
							auto distance = glm::distance(p1->pos * scale, p2->pos * scale);
							travelDistance = glm::distance(p1->pos, camera.getGlobalPosition()) / kHalfDim;
							auto minTravelTime = travelDistance / parameters.rendering.travelMaxSpeed;
							auto annihilationPartners = std::count_if(p1->potentialInteractionPartners.begin(), p1->potentialInteractionPartners.end(), [&](nm::Particle * partner){
								return (p1->getAnnihilationFlag() ^ partner->getAnnihilationFlag()) == 0xFF;
							});
							auto aproxAnnihilationTime = annihilationPartners * 1.f / environment->systemSpeed;

							bool foundNew = distance < nm::Octree<nm::Particle>::INTERACTION_DISTANCE();
							foundNew &= distance > nm::Octree<nm::Particle>::INTERACTION_DISTANCE() * 1. / 2.;
							foundNew &= ((p1->isMatterQuark() && p2->isAntiMatterQuark()) || (p1->isAntiMatterQuark() && p2->isMatterQuark()));
							foundNew &= minTravelTime < aproxAnnihilationTime * 0.8;
							foundNew &= p1->anihilationRatio + p2->anihilationRatio > 0.2;
							foundNew &= p1->pos.x > -kHalfDim * 0.5 && p1->pos.x < kHalfDim * 0.5;
							foundNew &= p1->pos.y > -kHalfDim * 0.5 && p1->pos.y < kHalfDim * 0.5;
							foundNew &= p1->pos.z > -kHalfDim * 0.5 && p1->pos.z < kHalfDim * 0.5;

							if(foundNew){
								if(p1->id<p2->id){
									lookAt.first = p1->id;
									lookAt.second = p2->id;
								}else{
									lookAt.first = p2->id;
									lookAt.second = p1->id;
								}
								timeRenewLookAt = now;
								prevLookAt = lerpedLookAt;
								prevCameraPosition = camera.getGlobalPosition();
								arrived = false;
								timeConnectionLost = 0;
								rotationDirection = round(ofRandomf());
								// cout << "renewed to " << p1->id << "  " << p2->id << " " << &p1 << " " << p2 << " at distance " << distance << endl;
								break;
							}
						}
					}
				break;
				case nm::Environment::STANDARD_MODEL:
					for(auto * p1: sortedParticles){
						auto * p2 = p1->fusionPartners.first;
						auto * p3 = p1->fusionPartners.second;
						if(p2 && p3){
							auto distance = glm::distance(p1->pos * scale, p2->pos * scale);
							travelDistance = glm::distance(p1->pos, camera.getGlobalPosition()) / kHalfDim;
							auto minTravelTime = travelDistance / parameters.rendering.travelMaxSpeed;
							auto fussionPartners = std::count_if(p1->potentialInteractionPartners.begin(), p1->potentialInteractionPartners.end(), [&](nm::Particle * partner){
								return (p1->getFusion1Flag() ^ partner->getFusion1Flag()) == 0xFF ||
									   (p1->getFusion2Flag() ^ partner->getFusion2Flag()) == 0xFF;
							});
							auto aproxFussionTime = fussionPartners * 1.f / environment->systemSpeed;

							bool foundNew = true;//distance < nm::Octree<nm::Particle>::INTERACTION_DISTANCE();
//							foundNew &= distance > nm::Octree<nm::Particle>::INTERACTION_DISTANCE() * 1. / 2.;
//							foundNew &= minTravelTime < aproxFussionTime * 0.8;
							//foundNew &= p1->anihilationRatio + p2->anihilationRatio > 0.2;
							foundNew &= p1->pos.x > -kHalfDim * 0.5 && p1->pos.x < kHalfDim * 0.5;
							foundNew &= p1->pos.y > -kHalfDim * 0.5 && p1->pos.y < kHalfDim * 0.5;
							foundNew &= p1->pos.z > -kHalfDim * 0.5 && p1->pos.z < kHalfDim * 0.5;

							if(foundNew){
								if(p1->id<p2->id){
									lookAt.first = p1->id;
									lookAt.second = p2->id;
								}else{
									lookAt.first = p2->id;
									lookAt.second = p1->id;
								}
								timeRenewLookAt = now;
								prevLookAt = lerpedLookAt;
								prevCameraPosition = camera.getGlobalPosition();
								timeConnectionLost = 0;
								rotationDirection = round(ofRandomf());
								if(parameters.rendering.cutToInteraction){
									arrived = true;
									lerpedLookAt = (p1->pos + p2->pos + p3->pos)/3.f;
								}else{
									arrived = false;
								}
								// cout << "renewed to " << p1->id << "  " << p2->id << " " << &p1 << " " << p2 << " at distance " << distance << endl;
								break;
							}
						}
					}
				break;
				case nm::Environment::NUCLEOSYNTHESIS:
				break;

			}

				//cout << "renewed " << !arrived << endl;
		}

		if(!arrived){
			auto * lookAt1 = particleSystem.getById(lookAt.first);
			auto * lookAt2 = particleSystem.getById(lookAt.second);
			if(lookAt1 && lookAt2){
				lookAtPos = (lookAt1->pos + lookAt2->pos) / 2.;
			}
			currentLookAtParticles.first = lookAt1;
			currentLookAtParticles.second = lookAt2;

			auto timePassed = now - timeRenewLookAt;
			auto animationTime = travelDistance / parameters.rendering.travelMaxSpeed;
			auto pct = float(timePassed) / animationTime;
			if(pct>1){
				pct = 1;
				if(!arrived){
					arrived = true;
				}
			}
			pct = ofxeasing::map(pct, 0, 1, 0, 1, ofxeasing::quad::easeInOut);
			lerpedLookAt = glm::lerp(prevLookAt, lookAtPos, pct);


	//		auto newCameraPosition = glm::lerp(prevCameraPosition, lookAtPos + glm::vec3(0,0,50), pct);

	//		camera.setPosition(newCameraPosition);
	//		camera.lookAt(lerpedLookAt, glm::vec3(0,1,0));
		}else{
			currentLookAtParticles.first = nullptr;
			currentLookAtParticles.second = nullptr;
	//		camera.orbitDeg(orbitAngle, 0, 50, lerpedLookAt);
	//		//	camera.orbitDeg(orbitAngle, 0, parameters.rendering.rotationRadius * kHalfDim);
	//		orbitAngle += dt * parameters.rendering.rotationSpeed * rotationDirection;
	//		orbitAngle = ofWrap(orbitAngle, 0, 360);
		}
	}

	if(environment->state<nm::Environment::TRANSITION_OUT){
		camera.orbitDeg(orbitAngle, 0, parameters.rendering.rotationRadius, lerpedLookAt);
		//	camera.orbitDeg(orbitAngle, 0, parameters.rendering.rotationRadius * kHalfDim);
		rotationSpeed = rotationSpeed * 0.9 + dt * parameters.rendering.rotationSpeed * rotationDirection * 0.1;
		orbitAngle += rotationSpeed;
		orbitAngle = ofWrap(orbitAngle, 0, 360);
	}

//	camera.setPosition(lookAtPos + glm::vec3(0,0,50));
//	camera.lookAt(lookAtPos, glm::vec3(0,1,0));



//	auto findNewAnnihilation = [this]{
//		std::pair<size_t, size_t> lookAt{0,0};
//		auto scale = environment->getExpansionScalar();
//		auto particles = particleSystem.getParticles();
//		std::vector<nm::Particle*> particlesPtr(particles.size());
//		std::transform(particles.begin(),
//					   particles.end(),
//					   particlesPtr.begin(),
//					   [](nm::Particle & p){ return &p; });

//		std::vector<nm::Particle*> sortedParticles;
//		std::copy_if(particlesPtr.begin(), particlesPtr.end(), std::back_inserter(sortedParticles),
//					 [](nm::Particle * p){ return p->isQuark(); });
//		std::sort(sortedParticles.begin(), sortedParticles.end(), [&](nm::Particle * p1, nm::Particle * p2){
//			return glm::distance2(p1->pos, camera.getGlobalPosition()) < glm::distance2(p2->pos, camera.getGlobalPosition());
//		});

//		std::vector<nm::Particle*> annihilationPartners;
//		for(auto * p1: sortedParticles){
//			annihilationPartners.clear();
//			std::copy_if(p1->potentialInteractionPartners.begin(), p1->potentialInteractionPartners.end(), std::back_inserter(annihilationPartners), [&](nm::Particle * partner){
//				return (p1->id < partner->id) && (p1->getAnnihilationFlag() ^ partner->getAnnihilationFlag()) == 0xFF;
//			});
//			for(auto * p2: annihilationPartners){
//				auto distance = glm::distance(p1->pos * scale, p2->pos * scale);
//				travelDistance = glm::distance(p1->pos, camera.getPosition());
//				auto minTravelTime = travelDistance / kHalfDim / parameters.rendering.travelMaxSpeed;
//				auto aproxAnnihilationTime = annihilationPartners.size() * 1.f / environment->systemSpeed;
//				travelSpeed = std::max(travelDistance / (aproxAnnihilationTime * 0.8f), parameters.rendering.travelMaxSpeed * kHalfDim);

//				bool foundNew = distance < nm::Octree<nm::Particle>::INTERACTION_DISTANCE();
//				foundNew &= distance > nm::Octree<nm::Particle>::INTERACTION_DISTANCE() * 1. / 2.;
//				foundNew &= minTravelTime < aproxAnnihilationTime * 0.5;

//				if(foundNew){
//					if(p1->id<p2->id){
//						lookAt.first = p1->id;
//						lookAt.second = p2->id;
//					}else{
//						lookAt.first = p2->id;
//						lookAt.second = p1->id;
//					}
//					break;
//				}
//			}
//		}
//		return lookAt;
//	};

//	auto distanceToTarget = glm::distance(camera.getPosition(), lookAtPos);
//	auto pctSpeed = travelDistance>0 ? ofMap(distanceToTarget / travelDistance, 0, 1, 2, 0.1, true) : 1;
//	if(lookAt.first != lookAt.second){
//		auto * lookAt1 = particleSystem.getById(lookAt.first);
//		if(lookAt1){
//			auto pctAnni = lookAt1->anihilationRatio / environment->getAnnihilationThresh();
//			cout << pctAnni << endl;
//		}
//	}
////	travelSpeed = /*travelSpeed * 0.9 + */parameters.rendering.travelMaxSpeed * kHalfDim * dt * pctSpeed/* * 0.1*/;
//	traveledLength += travelSpeed * dt;// * pctSpeed;

//	if(cameraPath.getVertices().empty() || traveledLength >= cameraPath.getPerimeter()){
//		lookAt = findNewAnnihilation();
//		if(lookAt.first != lookAt.second){
//			auto * lookAt1 = particleSystem.getById(lookAt.first);
//			auto * lookAt2 = particleSystem.getById(lookAt.second);
//			if(lookAt1 && lookAt2){
//				lookAtPos = (lookAt1->pos + lookAt2->pos) / 2.;
//				cameraViewport.setPosition(lookAtPos + glm::vec3(0,0,50));
//				cameraViewport.lookAt(lookAtPos, glm::vec3(0,1,0));
//				float nextDistance = glm::distance(camera.getPosition(), lookAtPos);
//				auto resolution = nextDistance * 10.;
//				cameraPath.curveTo(lookAtPos, resolution);
//				arrived = false;
//				pct = 0;
//				timeRenewLookAt = now;
//				cout << "renewed to " << lookAtPos << " at distance " << nextDistance << " with reolution " << resolution << endl;
//			}
//		}
//	}


//	if(traveledLength < cameraPath.getPerimeter()){
//		auto left = camera.getXAxis();
//		auto next = cameraPath.getPointAtLength(traveledLength);
//		auto nextLength = traveledLength + travelSpeed;
//		auto lookAt = cameraPath.getPointAtLength(nextLength);
//		auto up = glm::cross(left, glm::normalize(lookAt - next));
//		camera.setPosition(next);
//		camera.lookAt(lookAt, up);
//	}

//	cout << traveledLength << " / " << cameraPath.getPerimeter() <<  " @ " << pctSpeed << endl;


	if(environment->state == nm::Environment::TRANSITION_OUT){
		for(auto & cluster: clusters){
			if(dt>0){
				cluster.origin = cluster.origin * parameters.rendering.scaleFactor.get();
				cluster.scale = cluster.scale * parameters.rendering.scaleFactor;
				cluster.alpha = ofMap(cluster.scale, 0.01, 0.8, 0, 1, true);
			}
		}

		clusters.erase(std::remove_if(clusters.begin(), clusters.end(),
									  [&](Cluster & cluster){ return cluster.scale < 0.01; }),
				clusters.end());
	}
}

//--------------------------------------------------------------
void ofApp::renderScene(ofFbo & fboScene, ofFbo & fboPost, TextRenderer & textRenderer)
{
	fboScene.begin();
	if(parameters.rendering.trailsAlpha<1){
		ofMesh clearRect;
		clearRect.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
		clearRect.addVertex({0, 0, 0});
		clearRect.addVertex({fboScene.getWidth(), 0, 0});
		clearRect.addVertex({fboScene.getWidth(), fboScene.getHeight(), 0});
		clearRect.addVertex({0, fboScene.getHeight(), 0});
		clearRect.getColors().assign(4, ofFloatColor(0, parameters.rendering.trailsAlpha));
		clearRect.draw();
	}else{
		ofClear(0, 255);
	}
	if(parameters.rendering.drawText){
		ofEnableBlendMode(OF_BLENDMODE_ADD);
		if(parameters.rendering.useEasyCam){
			textRenderer.renderLines(particleSystem,
							  photonsAlive,
							  *environment,
							  renderer,
							  easyCam);
		}else{
			textRenderer.renderLines(particleSystem,
							  photonsAlive,
							  *environment,
							  renderer,
							  camera);
		}
	}
	fboScene.end();

	// Draw the scene.
	fboScene.begin();
	{

		if(parameters.rendering.useEasyCam){
			easyCam.begin();
		}else{
			camera.begin();
		}
		ofEnableDepthTest();
		{
			if (this->parameters.debugLights)
			{
				for (auto& light : pointLights)
				{
					if (light.getPosition().x > glm::vec3(-kHalfDim).x  &&
						light.getPosition().y > glm::vec3(-kHalfDim).y  &&
						light.getPosition().z > glm::vec3(-kHalfDim).z  &&
						light.getPosition().x < glm::vec3(kHalfDim).x &&
						light.getPosition().y < glm::vec3(kHalfDim).y &&
						light.getPosition().z < glm::vec3(kHalfDim).z)
					{
						light.draw();
					}
				}
			}



			if(parameters.rendering.drawText){
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				if(parameters.rendering.useEasyCam){
					textRenderer.draw(particleSystem,
									  photonsAlive,
									  *environment,
									  currentLookAtParticles,
									  renderer,
									  easyCam);
				}else{
					textRenderer.draw(particleSystem,
									  photonsAlive,
									  *environment,
									  currentLookAtParticles,
									  renderer,
									  camera);
				}
			}



			if (this->parameters.rendering.additiveBlending){
				ofEnableBlendMode(OF_BLENDMODE_ADD);
			}else if(this->parameters.rendering.glOneBlending){
				glBlendFunc(GL_ONE, GL_ONE);
			}else{
				ofEnableAlphaBlending();
			}
			if(parameters.rendering.drawModels){
				if(environment->state != nm::Environment::TRANSITION_OUT){
					if(parameters.rendering.useEasyCam){
						this->renderer.draw(feedbackVbo, 0, numPrimitives * 3, GL_TRIANGLES, easyCam);
					}else{
						this->renderer.draw(feedbackVbo, 0, numPrimitives * 3, GL_TRIANGLES, camera);
					}
				}else{
					float alpha = renderer.parameters.alphaFactor;
					bool first = true;
//					for(int i=0;i<1;i++){
//						auto & cluster = clusters[i];

					for(auto & cluster: clusters){
						ofNode node;
						node.setScale(cluster.scale);
						node.setPosition(cluster.origin);
						if(!first){
							//node.lookAt(camera.getPosition(), glm::vec3(0,1,0));
							node.setOrientation(cluster.rotation);
						}
						first = false;
						renderer.parameters.alphaFactor = alpha * cluster.alpha * parameters.rendering.particlesAlpha;
						this->renderer.draw(feedbackVbo, 0, numPrimitives * 3, GL_TRIANGLES, camera, node.getLocalTransformMatrix());
					}
					renderer.parameters.alphaFactor = alpha;
				}
			}
			if (parameters.rendering.drawPhotons)
			{
				photons.draw();
			}
		}
		ofDisableDepthTest();

		//cameraPath.draw();

		if(parameters.rendering.useEasyCam){
			camera.draw();
			easyCam.end();
		}else{
			camera.end();
		}

//		ofFill();
//		ofSetColor(0);
//		ofDrawRectangle({fboScene.getWidth() - 320, fboScene.getHeight(), 320, -240});
//		ofSetColor(255);
//		cameraViewport.begin({fboScene.getWidth() - 320, 0, 320, 240});
//		ofEnableBlendMode(OF_BLENDMODE_ADD);
//		textRenderer.draw(particleSystem, *environment, BARYOGENESIS, currentLookAtParticles, renderer, cameraViewport);
//		if (this->parameters.rendering.additiveBlending){
//			ofEnableBlendMode(OF_BLENDMODE_ADD);
//		}else if(this->parameters.rendering.glOneBlending){
//			glBlendFunc(GL_ONE, GL_ONE);
//		}else{
//			ofEnableAlphaBlending();
//		}
//		this->renderer.draw(feedbackVbo, 0, numPrimitives * 3, GL_TRIANGLES, cameraViewport);
//		cameraViewport.end();
	}
	fboScene.end();

	this->postEffects.process(fboScene.getTexture(), fboPost, this->postParams);
}

void ofApp::draw(){
	ofDisableBlendMode();
	ofSetColor(ofColor::white);
	if(parameters.frontBack.front){
		renderScene(fboSceneFront, fboPostFront, textRendererFront);
		auto h = ofGetWidth() * fboPostFront.getHeight() / fboPostFront.getWidth();
		fboPostFront.draw(0, 0, ofGetWidth(), h);
	}
	if(parameters.frontBack.back){
		renderScene(fboPostBack, fboPostBack, textRendererBack);
		auto h = ofGetWidth() * fboPostBack.getHeight() / fboPostBack.getWidth();
		fboPostBack.draw(0, 0, ofGetWidth(), h);
	}


	if (this->parameters.recording.recordSequence || this->parameters.recording.recordVideo)
	{
		this->textureRecorderFront.save(this->fboPostFront.getTexture());
		this->textureRecorderBack.save(this->fboPostBack.getTexture());

		if (this->timeline.getCurrentFrame() == this->timeline.getOutFrame())
		{
			this->parameters.recording.recordSequence = false;
			this->parameters.recording.recordVideo = false;
		}
	}

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	// Draw the controls.
	this->timeline.draw();
	this->gui.draw();

	ofSetColor(0,120);
	ofDrawRectangle(ofGetWidth() - 300, 0, 300, 500);
	ofSetColor(255);
	ofDrawBitmapString(ofGetFrameRate(), ofGetWidth()-100, 20);
	ofDrawBitmapString(particleSystem.getStatsStr(), ofGetWidth() - 280, 40);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
//	if (key == 'L')
//	{
//		this->cameraTrack.lockCameraToTrack ^= 1;
//	}
//	else if (key == 'T')
//	{
//		this->cameraTrack.addKeyframe();
//	}

	if(key == 'f'){
		ofToggleFullscreen();
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
void ofApp::windowResized(int w, int h)
{
	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void ofApp::reset()
{
	particleSystem.clearParticles();
	
	ofSetGlobalAmbientColor(ofFloatColor(parameters.rendering.ambientLight));

	// Use the same random seed for each run.
	ofSeedRandom(3030);

	int counts[6];
	for (int i = 0; i < 6; ++i)
	{
		counts[i] = 0;
	}

	for(size_t i=0;i<particleTypesInitialNumbers.size();i++){
		for (int j = 0; j < particleTypesInitialNumbers[i]; ++j)
		{
			glm::vec3 position = glm::vec3(
				ofRandom(-kHalfDim, kHalfDim),
				ofRandom(-kHalfDim, kHalfDim),
				ofRandom(-kHalfDim, kHalfDim)
			);

			float speed = glm::gaussRand(60.f, 20.f);
			glm::vec3 velocity = glm::sphericalRand(speed);

			particleSystem.addParticle((nm::Particle::Type)i, position, velocity);

			//particleSystem.addParticle((nm::Particle::Type)(i % 6), position, velocity);
		}
	}
}

//--------------------------------------------------------------
bool ofApp::loadPreset(const string & presetName)
{
	// Make sure file exists.
	const auto presetPath = std::filesystem::path("presets") / presetName;
	const auto presetFile = ofFile(presetPath);
	if (presetFile.exists())
	{
		// Load parameters from the preset.
		const auto paramsPath = presetPath / "parameters.json";
		const auto paramsFile = ofFile(paramsPath);
		if (paramsFile.exists())
		{
			const auto json = ofLoadJson(paramsFile);
			ofDeserialize(json, this->gui.getParameter());
			//ofDeserialize(json, this->camera, "ofEasyCam");
		}

		this->timeline.loadStructure(presetPath.string());
		this->timeline.loadTracksFromFolder(presetPath.string());

		this->gui.refreshTimelined(&this->timeline);

		this->currPreset = presetName;
	}
	else
	{
		ofLogWarning(__FUNCTION__) << "Directory not found at path " << presetPath;
		this->currPreset.clear();
	}

	// Setup scene with the new parameters.
	this->reset();

	if (this->currPreset.empty())
	{
		return false;
	}

	return true;
}

//--------------------------------------------------------------
bool ofApp::savePreset(const string & presetName)
{
	const auto presetPath = std::filesystem::path("presets") / presetName;

	const auto paramsPath = presetPath / "parameters.json";
	nlohmann::json json;
	ofSerialize(json, this->gui.getParameter());
	ofSerialize(json, this->camera, "ofEasyCam");
	ofSavePrettyJson(paramsPath, json);

	this->timeline.saveTracksToFolder(presetPath.string());
	this->timeline.saveStructure(presetPath.string());

	return true;
}
