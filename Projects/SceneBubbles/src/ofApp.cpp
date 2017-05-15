#include "ofApp.h"
#include "ofxSerialize.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	// Init the pools.
	this->pool2D.setDimensions(glm::vec2(ofGetWidth(), ofGetHeight()));
	this->pool2D.init();

	this->pool3D.setDimensions(glm::vec3(256.0f));
	this->pool3D.init();

	// Init the sphere.
	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Bubbles", "images/texture-CMB-2.png"), this->sphereTexture);

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

	const auto cameraTrackName = "Camera";
	this->cameraTrack.setDampening(1.0f);
	this->cameraTrack.setCamera(this->camera);
	this->cameraTrack.setXMLFileName(this->timeline.nameToXMLName(cameraTrackName));
	this->timeline.addTrack(cameraTrackName, &this->cameraTrack);
	this->cameraTrack.setDisplayName(cameraTrackName);

	this->gui.setTimeline(&this->timeline);
	//this->gui.loadFromFile("parameters.json");

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
			this->boxGeom.draw();

			this->sphereShader.begin();
			{
				this->sphereShader.setUniformTexture("uTexColor", this->sphereTexture, 1);
				this->sphereShader.setUniformTexture("uTexMask", this->pool3D.getTexture().texData.textureTarget, this->pool3D.getTexture().texData.textureID, 2);
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
		}
		ofDisableDepthTest();
		this->camera.end();
	}
	this->fboScene.end();

	this->postEffects.process(this->fboScene.getTexture(), this->fboPost, this->postParams);

	ofDisableBlendMode();
	ofSetColor(ofColor::white);
	this->fboPost.draw(0, 0);

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
	this->pool2D.setDimensions(glm::vec2(ofGetWidth(), ofGetHeight()));

	this->camera.setAspectRatio(ofGetWidth() / static_cast<float>(ofGetHeight()));

	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));

	auto fboSettings = ofFbo::Settings();
	fboSettings.width = ofGetWidth();
	fboSettings.height = ofGetHeight();
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

	this->timeline.setCurrentFrame(0);
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
		this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));

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
