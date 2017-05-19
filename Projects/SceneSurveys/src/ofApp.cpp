#include "ofApp.h"
#include "ofxSerialize.h"

//--------------------------------------------------------------
const string ofApp::kSceneName = "Surveys";

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofSetDataPathRoot(entropy::GetSceneDataPath("Surveys").string());
	ofBackground(ofColor::black);

	// Load the data.
	this->dataSetBoss.setup("BOSS", entropy::GetSceneAssetPath("Surveys", "/particles/boss_fragment-batch-%iof10.hdf5").string(), 0, 10, "PartType6");
	this->dataSetDes.setup("DES", entropy::GetSceneAssetPath("Surveys", "particles/des_fragment-batch-%iof20.hdf5").string(), 0, 20, "PartType6");
	this->dataSetVizir.setup("ViziR", entropy::GetSceneAssetPath("Surveys", "particles/Hipparchos-Tycho-stars-fromViziR.hdf5").string(), 0, 1, "PartType4");

	// Init the sphere.
	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Surveys", "images/The_Milky_Way.png"), this->sphereTexture);

	this->sphereSettings = ofShader::Settings();
	this->sphereSettings.bindDefaults = true;
	this->sphereSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/galaxy.vert";
	this->sphereSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/galaxy.frag";
	this->sphereShader.setup(this->sphereSettings);

	// Build the textures.
	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Surveys", "images/sprites.png"), this->spriteTexture);
	entropy::LoadTextureImage("textures/ring.png", this->shellTexture);

	// Load the model.
	this->masterMesh.load("models/galaxy.ply");
	this->masterMesh.setMode(OF_PRIMITIVE_POINTS);
	std::vector<ofFloatColor> colors{
		ofFloatColor::white,
		ofFloatColor::darkBlue,
		ofFloatColor::white,
		ofFloatColor::cyan,
		ofFloatColor::lightBlue,
		ofFloatColor::aliceBlue,
	};
	for (auto & v : this->masterMesh.getVertices()) 
	{
		auto d = glm::length2(v);
		v.y *= 0.8;
		auto pct = ofMap(d, 0, 0.85, 1, 0, true);
		auto bri = ofRandom(1) > 0.99 ? 35 : 15;
		this->masterMesh.addColor(colors[int(ofRandom(colors.size()))] * bri * pct);
	}

	this->scaledMesh = this->masterMesh;

	this->eventListeners.push_back(this->sharedParams.model.resolution.newListener([this](int & val)
	{
		this->scaledMesh.clear();
		if (val == 1)
		{
			this->scaledMesh = this->masterMesh;
		}
		else
		{
			for (int i = 0; i < this->masterMesh.getNumVertices(); i += val)
			{
				this->scaledMesh.addVertex(this->masterMesh.getVertex(i));
				this->scaledMesh.addColor(this->masterMesh.getColor(i));
			}
		}
	}));
	this->eventListeners.push_back(this->sharedParams.model.clipSize.newListener([this](float & val)
	{
		this->sharedParams.model.maxDensitySize.setMin(val + 0.001f);
	}));

	// Load the shaders.
	this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
	this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::Mass, "mass");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
	this->spriteShader.bindDefaults();
	this->spriteShader.linkProgram();

	this->shellSettings = ofShader::Settings();
	this->shellSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/shell.vert";
	this->shellSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/shell.frag";
	this->shellSettings.bindDefaults = true;
	this->shellShader.setup(this->shellSettings);

	this->modelSettings = ofShader::Settings();
	this->modelSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/instanced.vert";
	this->modelSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/instanced.frag";
	this->modelSettings.bindDefaults = true;
	this->modelShader.setup(this->modelSettings);

	// Setup the camera.
	this->eventListeners.push_back(this->parameters.camera.nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->eventListeners.push_back(this->parameters.camera.farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
	}));
	this->eventListeners.push_back(this->parameters.camera.fov.newListener([this](float & val)
	{
		this->camera.setFov(val);
	}));

	this->eventListeners.push_back(this->parameters.travel.enabled.newListener([this](bool &)
	{
		this->prevTargetIndex = -1;
		this->travelLog.clear();
	}));

	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("FiraCode-Light", 11, true, true, 72);
	this->gui.setup("Surveys", "parameters.json");
	this->gui.add(this->parameters);
	this->gui.add(this->sharedParams);
	this->gui.add(this->sphereGeom.parameters);
	this->gui.add(this->dataSetBoss.parameters);
	this->gui.add(this->dataSetDes.parameters);
	this->gui.add(this->dataSetVizir.parameters);
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

	this->guiVisible = true;
	this->timelineVisible = true;

	// Setup texture recorder.
	this->eventListeners.push_back(this->parameters.recording.recordSequence.newListener([this](bool & record)
	{
		if (record)
		{
			auto path = ofSystemLoadDialog("Record to folder:", true);
			if (path.bSuccess)
			{
				// Resize canvas.
				this->windowResized(this->parameters.recording.renderWidth, this->parameters.recording.renderHeight);

				// Setup texture recorder.
				ofxTextureRecorder::Settings recorderSettings(this->fboPost.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path.getPath();
				this->textureRecorder.setup(recorderSettings);

				ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));

				// Start scene.
				this->reset();
				this->cameraTrack.lockCameraToTrack = true;
				this->timeline.play();
			}
			else
			{
				// Resize canvas.
				this->windowResized(ofGetWidth(), ofGetHeight()); 

				ofSetTimeModeSystem();
				
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
				ofxTextureRecorder::VideoSettings recorderSettings(fbo.getTexture(), 60);
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

glm::vec3 spherical;

//--------------------------------------------------------------
void ofApp::update()
{
	this->orbitOffset.y += this->parameters.orbitSpeed;

	/*
	if (this->orbitSpeed != 0.0f)
	{
		{
			const auto camPos = this->camera.getPosition();
			//auto radius = glm::length(camPos);

			//auto longitude = atan2(camPos.y, camPos.x);
			//auto latitude = atan2(glm::length(camPos.xy()), camPos.z);
			//auto latitude = acos(camPos.z / radius);
			//cout << "Orbit SRC " << radius << ", " << longitude << ", " << latitude << endl;
			
			//this->camera.orbitRad(latitude, longitude + ofDegToRad(this->orbitSpeed), radius);
			
			//latitude += ofDegToRad(this->orbitSpeed);
			//float x = radius * sin(latitude) * cos(longitude);
			//float y = radius * sin(latitude) * sin(longitude);
			//float z = radius * cos(latitude);

			//this->camera.setPosition(x, y, z);
			//this->camera.lookAt(glm::vec3(0));
			//cout << "Orbit DST " << radius << ", " << (longitude + ofDegToRad(this->orbitSpeed)) << ", " << latitude << endl;
		
		
			//spherical.x = glm::length(camPos) * 0.5f;
			//spherical.y += ofDegToRad(this->orbitSpeed);
			//this->camera.orbitRad(spherical.y, spherical.z, spherical.x);
		}
		//{
		//	const auto camPos = this->camera.getPosition();
		//	auto radius = glm::length(camPos);
		//	auto longitude = atan2(camPos.y, camPos.x);
		//	auto latitude = atan2(glm::length(camPos.xy()), camPos.z);
		//	cout << "Orbit RES " << radius << ", " << longitude << ", " << latitude << endl;

		//}
		this->camera.rotateAroundRad(ofDegToRad(this->orbitSpeed), glm::vec3(0, 1, 0), glm::vec3(0));
		this->camera.lookAt(glm::vec3(0));
	}
	*/

	if (this->parameters.travel.enabled)
	{
		// Get the data target.
		int currTargetIndex = this->dataSetDes.getTargetIndex();
		glm::vec3 targetPos;
		const auto camPos = this->camera.getPosition();
		if (glm::length(camPos) > this->parameters.travel.camCutoff &&
			(this->prevTargetIndex == currTargetIndex ||
			 this->travelLog.find(currTargetIndex) == this->travelLog.end()))
		{
			// Camera is far enough from the origin.
			// Target is either the same as previous frame, or it's never been visited on this travel.
			targetPos = this->dataSetDes.getTargetPosition() * this->parameters.worldScale.get();
			this->prevTargetIndex = currTargetIndex;
			this->travelLog.insert(currTargetIndex);
			cout << "[ofApp] Travel target index = " << this->prevTargetIndex << endl;
		}
		else
		{
			// Using default target at origin.
			this->prevTargetIndex = -1;
			cout << "[ofApp] Default target index = " << this->prevTargetIndex << endl;
		}
		
		const auto toTarget = targetPos - camPos;
		const auto lerpedDir = glm::normalize(glm::mix(this->camera.getLookAtDir(), glm::normalize(toTarget), this->parameters.travel.lookAtLerp.get()));
		const auto targetDist = glm::length(toTarget);

		this->camera.lookAt(camPos + lerpedDir * targetDist);
		auto lerpPos = glm::mix(camPos, targetPos, this->parameters.travel.moveLerp.get());
		if (this->parameters.travel.maxSpeed > 0.0f && this->parameters.travel.maxSpeed < glm::distance(camPos, lerpPos))
		{
			// Clamp the new position if it's too big of a jump.
			lerpPos = camPos + glm::normalize(toTarget) * this->parameters.travel.maxSpeed.get();
		}
		this->camera.setPosition(lerpPos);
	}
	
	// Auto-reload shaders.
	auto vertTime = std::filesystem::last_write_time(ofToDataPath("shaders/galaxy.vert"));
	auto fragTime = std::filesystem::last_write_time(ofToDataPath("shaders/galaxy.frag"));
	if (vertTime > sphereTime || fragTime > sphereTime)
	{
		sphereTime = std::max(vertTime, fragTime);
		this->sphereShader.setup(sphereSettings);
	}

	vertTime = std::filesystem::last_write_time(ofToDataPath("shaders/sprite.vert"));
	fragTime = std::filesystem::last_write_time(ofToDataPath("shaders/sprite.frag"));
	if (vertTime > spriteTime || fragTime > spriteTime)
	{
		spriteTime = std::max(vertTime, fragTime);
		this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
		this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
		this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::Mass, "mass");
		this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
		this->spriteShader.bindDefaults();
		this->spriteShader.linkProgram();
	}

	vertTime = std::filesystem::last_write_time(ofToDataPath("shaders/shell.vert"));
	fragTime = std::filesystem::last_write_time(ofToDataPath("shaders/shell.frag"));
	if (vertTime > shellTime || fragTime > shellTime)
	{
		shellTime = std::max(vertTime, fragTime);
		this->shellShader.setup(shellSettings);
	}

	vertTime = std::filesystem::last_write_time(ofToDataPath("shaders/instanced.vert"));
	fragTime = std::filesystem::last_write_time(ofToDataPath("shaders/instanced.frag"));
	if (vertTime > modelTime || fragTime > modelTime)
	{
		modelTime = std::max(vertTime, fragTime);
		this->modelShader.setup(modelSettings);
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw the scene.
	this->fboScene.begin();
	{
		ofClear(0, 255);

		this->camera.begin();
		{
			//renderer.draw(galaxy.getVbo(), 0, galaxy.getNumVertices(), GL_POINTS, camera);

			auto worldTransform = this->getWorldTransform();

			// Draw the data set.
			{
				//glEnable(GL_BLEND);
				//glBlendFunc(GL_ONE, GL_ONE);
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				// Draw all the points.
				ofSetColor(ofColor::white);

				this->spriteShader.begin();
				this->spriteShader.setUniform2f("uClipRange", this->camera.getFarClip() * (1.0f - this->sharedParams.point.distanceFade), this->camera.getFarClip());
				this->spriteShader.setUniform1f("uPointSize", this->sharedParams.point.size);
				this->spriteShader.setUniform1f("uAttenuation", this->sharedParams.point.attenuation);
				this->spriteShader.setUniformMatrix4f("uTransform", worldTransform);
				this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
				ofEnablePointSprites();
				{
					this->dataSetBoss.drawPoints(this->spriteShader, this->sharedParams);

					this->dataSetDes.drawPoints(this->spriteShader, this->sharedParams);

					this->dataSetVizir.drawPoints(this->spriteShader, this->sharedParams);
				}
				ofDisablePointSprites();
				this->spriteShader.end();

				// Draw all the shells.
				ofSetColor(ofColor::white, this->sharedParams.shell.alpha * 255);

				this->shellShader.begin();
				this->shellShader.setUniform1f("uPointSize", this->sharedParams.shell.size);
				this->shellShader.setUniform1f("uAttenuation", this->sharedParams.shell.attenuation);
				this->shellShader.setUniform1f("uDensity", this->sharedParams.shell.density);
				this->shellShader.setUniformMatrix4f("uTransform", worldTransform);
				this->shellShader.setUniformTexture("uTex0", this->shellTexture, 1);
				ofEnablePointSprites();
				{
					this->dataSetBoss.drawShells(this->shellShader, this->sharedParams);
					this->dataSetDes.drawShells(this->shellShader, this->sharedParams);
				}
				ofDisablePointSprites();
				this->shellShader.end();

				// Draw the models for galaxies near the camera.
				ofSetColor(ofColor::white);

				this->modelShader.begin();
				{
					this->dataSetBoss.drawModels(this->modelShader, this->sharedParams, worldTransform, this->scaledMesh, this->camera);
					this->dataSetDes.drawModels(this->modelShader, this->sharedParams, worldTransform, this->scaledMesh, this->camera);
				}
				this->modelShader.end();
			}

			//ofNoFill();
			//const auto targetPos = this->dataSetDes.getTargetPosition() * this->parameters.worldScale.get();
			//ofSetColor(ofColor::red);
			//ofDrawLine(glm::vec3(0), targetPos);
			//ofDrawBox(targetPos, this->sharedParams.target.lockDistance * 2.0f);
			//ofFill();
			
			ofPushMatrix();
			ofMultMatrix(worldTransform);
			{
				// Draw the galaxy texture.
				this->sphereShader.begin();
				{
					this->sphereShader.setUniformMatrix4f("uNormalMatrix", ofGetCurrentNormalMatrix());
					//this->sphereShader.setUniform1f("uRadius", this->sphereGeom.radius);
					this->sphereShader.setUniformTexture("uTex0", this->sphereTexture, 1);
					this->sphereShader.setUniform1f("uAlphaBase", this->sphereGeom.alpha);

					this->sphereGeom.draw();
				}
				this->sphereShader.end();
			}
			ofPopMatrix();
		}
		this->camera.end();
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
	if (this->timelineVisible)
	{
		this->timeline.draw();
	}
	if (this->guiVisible)
	{
		this->gui.draw();
	}
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
	else if (key == 'G')
	{
		this->guiVisible ^= 1;
	}
	else if (key == 'F')
	{
		this->timelineVisible ^= 1;
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
	this->orbitOffset = glm::vec3(0.0f);
	
	this->timeline.setCurrentFrame(0);
}

//--------------------------------------------------------------
glm::mat4 ofApp::getWorldTransform() const
{
	static const auto yAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	
	glm::mat4 transform;

	// Scale.
	transform = glm::scale(glm::mat4(1.0f), glm::vec3(this->parameters.worldScale));

	// Tumble.
	transform = glm::rotate(transform, ofDegToRad(this->orbitOffset.y), yAxis);

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
