#include "ofApp.h"
#include "ofxSerialize.h"
#include "Helpers.h"

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
	fboSettings.width = 1920;
	fboSettings.height = entropy::GetSceneHeight() * fboSettings.width / entropy::GetSceneWidth();
	fboSettings.internalformat = GL_RGBA32F;
	fboSettings.numSamples = 4;

	fboScene.allocate(fboSettings);
	fboPost.allocate(fboSettings);

	// Init post effects
	this->postEffects.resize(fboSettings.width, fboSettings.height);
	this->renderer.resize(fboSettings.width, fboSettings.height);

	// Setup renderers.
	this->renderer.setup(kHalfDim);
	this->renderer.parameters.fogMaxDistance.setMax(kHalfDim * 100);
	this->renderer.parameters.fogMinDistance.setMax(kHalfDim);

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
	this->gui.add(nm::Particle::parameters);
	this->gui.add(this->renderer.parameters);
	this->gui.add(this->postParams);
	this->gui.minimizeAll();
//	this->eventListeners.push_back(this->gui.savePressedE.newListener([this](void)
//	{
//		if (ofGetKeyPressed(OF_KEY_SHIFT))
//		{
//			auto name = ofSystemTextBoxDialog("Enter a name for the preset", "");
//			if (!name.empty())
//			{
//				return this->savePreset(name);
//			}
//		}
//		return this->savePreset(this->currPreset);
//	}));
//	this->eventListeners.push_back(this->gui.loadPressedE.newListener([this](void)
//	{
//		if (ofGetKeyPressed(OF_KEY_SHIFT))
//		{
//			auto result = ofSystemLoadDialog("Select a preset folder.", true, ofToDataPath("presets", true));
//			if (result.bSuccess)
//			{
//				return this->loadPreset(result.fileName);
//			}
//		}
//		return this->loadPreset(this->currPreset);
//	}));

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

	const auto cameraTrackName = "Camera";
	this->cameraTrack.setDampening(1.0f);
	this->cameraTrack.setCamera(this->camera);
	this->cameraTrack.setXMLFileName(this->timeline.nameToXMLName(cameraTrackName));
	this->timeline.addTrack(cameraTrackName, &this->cameraTrack);
	this->cameraTrack.setDisplayName(cameraTrackName);

	this->gui.setTimeline(&this->timeline);

	// Setup texture recorder.
	this->eventListeners.push_back(this->parameters.recording.recordSequence.newListener([this](bool & record)
	{
		if (record)
		{
			auto path = ofSystemLoadDialog("Record to folder:", true);
			if (path.bSuccess)
			{
				ofxTextureRecorder::Settings recorderSettings(this->fboPost.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path.getPath();
				this->textureRecorder.setup(recorderSettings);

				this->reset();
				this->cameraTrack.lockCameraToTrack = true;
				this->timeline.play();
			}
			else
			{
				this->parameters.recording.recordSequence = false;
			}
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
				ofxTextureRecorder::VideoSettings recorderSettings(fboScene.getTexture(), 60);
				recorderSettings.videoPath = path.getPath();
				//				recorderSettings.videoCodec = "libx264";
				//				recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				recorderSettings.videoCodec = "prores";
				recorderSettings.extrasettings = "-profile:v 0";
				textureRecorder.setup(recorderSettings);
			}
			else {
				this->parameters.recording.recordVideo = false;
			}
		}
		else {
			textureRecorder.stop();
		}
#endif
	}));

	eventListeners.push_back(gui.savePressedE.newListener([this]{
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


	eventListeners.push_back(gui.loadPressedE.newListener([this]{
		auto loadFrom = ofSystemLoadDialog("load settings", false, ofToDataPath("presets"));
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
			reset();
		}
		return true;
	}));

	eventListeners.push_back(parameters.recording.fps.newListener([this](int & fps){
		if(parameters.recording.systemClock){
			ofSetTimeModeSystem();
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(fps));
		}
	}));

	eventListeners.push_back(parameters.recording.systemClock.newListener([this](bool & systemClock){
		if(systemClock){
			ofSetTimeModeSystem();
		}else{
			ofSetTimeModeFixedRate(ofGetFixedStepForFps(parameters.recording.fps));
		}
	}));

	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	//this->loadPreset("_autosave");
	//this->reset();
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

	photons.update(dt);
	particleSystem.update(dt);

	auto & photons = this->photons.getPosnsRef();

	for (auto & light : pointLights)
	{
		light.disable();
		light.setPosition(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	}

	for (size_t i = 0, j = 0; i < photons.size() && j < kMaxLights; ++i)
	{
		if (photons[i].x > glm::vec3(-kHalfDim * 2).x  &&
			photons[i].y > glm::vec3(-kHalfDim * 2).y  &&
			photons[i].z > glm::vec3(-kHalfDim * 2).z  &&
			photons[i].x < glm::vec3(kHalfDim * 2).x &&
			photons[i].y < glm::vec3(kHalfDim * 2).y &&
			photons[i].z < glm::vec3(kHalfDim * 2).z) {
			auto & light = pointLights[j];
			light.enable();
			light.setDiffuseColor(ofFloatColor::white * parameters.rendering.lightStrength);
			light.setPointLight();
			light.setPosition(photons[i]);
			light.setAttenuation(0, 0, parameters.rendering.attenuation);
			j++;
		}
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
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw the scene.
	this->fboScene.begin();
	{
		ofClear(0, 255);

		camera.begin();
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
			else
			{
				if (this->parameters.rendering.additiveBlending)
				{
					ofEnableBlendMode(OF_BLENDMODE_ADD);
				}
				if(this->parameters.rendering.glOneBlending){
					glBlendFunc(GL_ONE, GL_ONE);
				}

				this->renderer.draw(feedbackVbo, 0, numPrimitives * 3, GL_TRIANGLES, camera, glm::mat4(1.0f));
				if (parameters.rendering.drawPhotons)
				{
					photons.draw();
				}
			}
		}
		ofDisableDepthTest();
		camera.end();
	}
	this->fboScene.end();

	this->postEffects.process(this->fboScene.getTexture(), this->fboPost, this->postParams);

	ofDisableBlendMode();
	ofSetColor(ofColor::white);
	this->fboPost.draw(0, 0);

	if (this->parameters.recording.recordSequence || this->parameters.recording.recordVideo)
	{
		this->textureRecorder.save(this->fboPost.getTexture());

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

	ofDrawBitmapString(ofGetFrameRate(), ofGetWidth()-100, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	if (key == 'L')
	{
		this->cameraTrack.lockCameraToTrack ^= 1;
	}
	else if (key == 'T')
	{
		this->cameraTrack.addKeyframe();
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

	for (unsigned i = 0; i < 4000; ++i)
	{
		glm::vec3 position = glm::vec3(
			ofRandom(-kHalfDim, kHalfDim),
			ofRandom(-kHalfDim, kHalfDim),
			ofRandom(-kHalfDim, kHalfDim)
		);

		float speed = glm::gaussRand(60.f, 20.f);
		glm::vec3 velocity = glm::sphericalRand(speed);

		particleSystem.addParticle((nm::Particle::Type)(i % 6), position, velocity);
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
			ofDeserialize(json, this->camera, "ofEasyCam");
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
