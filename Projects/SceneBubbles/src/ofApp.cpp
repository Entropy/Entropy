#include "ofApp.h"
#include "ofxSerialize.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofBackground(ofColor::black);
	ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
	
	// Init the pools.
	this->pool2D.setDimensions(glm::vec2(ofGetWidth(), ofGetHeight()));
	this->pool2D.init();

	this->pool3D.setDimensions(glm::vec3(256.0f));
	this->pool3D.init();

	// Init the sphere.
	this->sphereGeom.radius.setMax(8000.0f);

	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Bubbles", "images/texture-CMB-gray.png"), this->sphereTexture);

	auto shaderSettings = ofShader::Settings();
	shaderSettings.intDefines["USE_TEX_ARRAY"] = USE_TEX_ARRAY;
	shaderSettings.bindDefaults = true;
	shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/reveal.vert";
	shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/reveal.frag";
	this->sphereShader.setup(shaderSettings);

	// Setup the camera.
	this->eventListeners.push_back(this->parameters.camera.nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->eventListeners.push_back(this->parameters.camera.farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
	}));

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("FiraCode-Light", 11, true, true, 72);
	this->gui.setup("Bubbles", "parameters.json");
	this->gui.add(this->parameters);
	this->gui.add(this->boxGeom.parameters);
	this->gui.add(this->pool2D.parameters);
	this->gui.add(this->pool3D.parameters);
	this->gui.add(this->sphereGeom.parameters);
	this->gui.add(this->renderer.parameters);
	this->gui.add(this->postParams);
	this->gui.minimizeAll();
	this->eventListeners.push_back(this->gui.savePressedE.newListener([this](void)
	{
		if (ofGetKeyPressed(OF_KEY_SHIFT))
		{
			auto name = ofSystemTextBoxDialog("Enter a name for the preset", "");
			if (!name.empty())
			{
				return this->savePreset(name);
			}
		}
		return this->savePreset(this->currPreset);
	}));
	this->eventListeners.push_back(this->gui.loadPressedE.newListener([this](void)
	{
		if (ofGetKeyPressed(OF_KEY_SHIFT))
		{
			auto result = ofSystemLoadDialog("Select a preset folder.", true, ofToDataPath("presets", true));
			if (result.bSuccess)
			{
				return this->loadPreset(result.fileName);
			}
		}
		return this->loadPreset(this->currPreset);
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

	const auto cameraTrackName = "Camera";
	this->cameraTrack.setDampening(1.0f);
	this->cameraTrack.setCamera(this->camera);
	this->cameraTrack.setXMLFileName(this->timeline.nameToXMLName(cameraTrackName));
	this->timeline.addTrack(cameraTrackName, &this->cameraTrack);
	this->cameraTrack.setDisplayName(cameraTrackName);

	this->gui.setTimeline(&this->timeline);

	// Setup texture recorder.
	this->eventListeners.push_back(this->parameters.render.recordSequence.newListener([this](bool & record) 
	{
		if (record) 
		{
			auto path = ofSystemLoadDialog("Record to folder:", true);
			if (path.bSuccess) 
			{
				// Resize canvas.
				this->windowResized(this->parameters.render.renderWidth, this->parameters.render.renderHeight);
				
				// Setup texture recorder.
				ofxTextureRecorder::Settings recorderSettings(this->fboPost.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path.getPath();
				this->textureRecorder.setup(recorderSettings);

				// Start scene.
				this->reset();
				this->cameraTrack.lockCameraToTrack = true;
				this->timeline.play();
			}
			else 
			{
				// Resize canvas.
				this->windowResized(ofGetWidth(), ofGetHeight()); 
				
				this->parameters.render.recordSequence = false;
			}
		}
	}));

	this->eventListeners.push_back(parameters.render.recordVideo.newListener([this](bool & record) 
	{
#if OFX_VIDEO_RECORDER
		if (record)
		{
			auto path = ofSystemSaveDialog("video.mp4", "Record to video:");
			if (path.bSuccess) 
			{
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), 60);
				recorderSettings.videoPath = path.getPath();
				//				recorderSettings.videoCodec = "libx264";
				//				recorderSettings.extrasettings = "-preset ultrafast -crf 0";
				recorderSettings.videoCodec = "prores";
				recorderSettings.extrasettings = "-profile:v 0";
				recorder.setup(recorderSettings);
			}
			else {
				this->parameters.render.recordVideo = false;
			}
		}
		else {
			recorder.stop();
		}
#endif
	}));
	
	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	this->loadPreset("_autosave");
	//this->reset();
}

//--------------------------------------------------------------
void ofApp::exit()
{
	this->savePreset("_autosave");
}

//--------------------------------------------------------------
void ofApp::update()
{
	double dt = ofGetLastFrameTime();
	this->pool2D.update(dt);
	this->pool3D.update(dt);

	this->tumbleOffset.x += this->parameters.camera.tiltSpeed;
	this->tumbleOffset.y += this->parameters.camera.panSpeed;
	this->tumbleOffset.z += this->parameters.camera.rollSpeed;
	this->dollyOffset += this->parameters.camera.dollySpeed;
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw the scene.
	this->fboScene.begin();
	{
		ofClear(0, 255);

		if (this->pool2D.drawEnabled)
		{
			this->pool2D.draw();
		}

		this->camera.begin();
		ofEnableDepthTest();
		{
			ofPushMatrix();
			ofMultMatrix(this->getWorldTransform());

			this->boxGeom.draw();

			this->sphereShader.begin();
			{
				this->sphereShader.setUniformTexture("uTexColor", this->sphereTexture, 1);
				this->sphereShader.setUniformTexture("uTexMask", this->pool3D.getDrawTexture().texData.textureTarget, this->pool3D.getDrawTexture().texData.textureID, 2);
				this->sphereShader.setUniform3f("uMaskDims", this->pool3D.getDimensions());
				this->sphereShader.setUniform1f("uVolSize", this->pool3D.volumeSize);
				this->sphereShader.setUniform1f("uAlphaBase", this->sphereGeom.alpha);
				this->sphereShader.setUniform1f("uMaskMix", this->parameters.sphere.maskMix);
				this->sphereShader.setUniform4f("uTintColor", this->parameters.sphere.tintColor.get());

				this->sphereGeom.draw();
			}
			this->sphereShader.end();

			if (this->pool3D.drawEnabled)
			{
				this->pool3D.draw();
			}

			ofPopMatrix();
		}
		ofDisableDepthTest();
		this->camera.end();
	}
	this->fboScene.end();

	this->postEffects.process(this->fboScene.getTexture(), this->fboPost, this->postParams);

	ofDisableBlendMode();
	ofSetColor(ofColor::white);
	this->fboPost.draw(0, 0);

	if (this->parameters.render.recordSequence || this->parameters.render.recordVideo)
	{
		this->textureRecorder.save(this->fboPost.getTexture());

		if (this->timeline.getCurrentFrame() == this->timeline.getOutFrame())
		{
			this->parameters.render.recordSequence = false;
			this->parameters.render.recordVideo = false;
		}
	}

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	// Draw the controls.
	this->timeline.draw();
	this->gui.draw();
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
	int canvasWidth = w;
	int canvasHeight = h;

	this->pool2D.setDimensions(glm::vec2(canvasWidth, canvasHeight));

	this->camera.setAspectRatio(canvasWidth / static_cast<float>(canvasHeight));

	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));

	auto fboSettings = ofFbo::Settings();
	fboSettings.width = canvasWidth;
	fboSettings.height = canvasHeight;
	fboSettings.internalformat = GL_RGBA32F;
	fboSettings.textureTarget = GL_TEXTURE_2D;
	fboSettings.numSamples = 4;
	this->fboScene.allocate(fboSettings);
	this->fboPost.allocate(fboSettings);

	this->postEffects.resize(fboSettings.width, fboSettings.height);
	this->renderer.setup(1);
	this->renderer.resize(fboSettings.width, fboSettings.height);
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
	this->pool2D.reset();
	this->pool3D.reset();

	this->tumbleOffset = glm::vec3(0.0f);
	this->dollyOffset = 0.0f;

	this->timeline.setCurrentFrame(0);
}

//--------------------------------------------------------------
glm::mat4 ofApp::getWorldTransform() const
{
	static const auto xAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	static const auto yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	static const auto zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 transform;

	// Dolly.
	transform = glm::translate(transform, this->camera.getZAxis() * this->dollyOffset);

	// Tumble.
	transform = glm::rotate(transform, ofDegToRad(this->tumbleOffset.x), xAxis);
	transform = glm::rotate(transform, ofDegToRad(this->tumbleOffset.y), yAxis);
	transform = glm::rotate(transform, ofDegToRad(this->tumbleOffset.z), zAxis);

	return transform;
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
