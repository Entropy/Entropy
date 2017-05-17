#include "ofApp.h"

#include "entropy/Helpers.h"

//--------------------------------------------------------------
const string ofApp::kSceneName = "Surveys";

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofSetDataPathRoot(entropy::GetSceneDataPath("Surveys").string());
	//ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
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

	// Build the texture.
	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Surveys", "images/sprites.png"), this->spriteTexture);

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

	this->paramListeners.push_back(this->sharedParams.model.resolution.newListener([this](int & val)
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
	this->paramListeners.push_back(this->params.camera.nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->paramListeners.push_back(this->params.camera.farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
		this->sharedParams.model.clipDistance.setMax(val);
	}));

	this->paramListeners.push_back(this->params.travel.enabled.newListener([this](bool &)
	{
		this->prevTargetIndex = -1;
		this->travelLog.clear();
		cout << "Resetting travel log" << endl;
	}));

	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("FiraCode-Light", 11, true, true, 72);
	this->gui.setup("Surveys", "settings.json");
	this->gui.add(this->params);
	this->gui.add(this->sharedParams);
	this->gui.add(this->dataSetBoss.parameters);
	this->gui.add(this->dataSetDes.parameters);
	this->gui.add(this->dataSetVizir.parameters);
	this->gui.add(this->renderer.parameters);
	this->gui.add(this->postParams);
	this->gui.minimizeAll();

	this->timeline.setup();
	this->timeline.setDefaultFontPath("FiraCode-Light");
	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
	this->timeline.setDurationInSeconds(60 * 5);
	this->timeline.setAutosave(false);

	this->cameraTrack.setCamera(this->camera);
	this->timeline.addTrack("Camera", &this->cameraTrack);

	this->gui.setTimeline(&this->timeline);
	this->gui.loadFromFile("settings.json");
}

//--------------------------------------------------------------
void ofApp::exit()
{

}

glm::vec3 spherical;

//--------------------------------------------------------------
void ofApp::update()
{
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
	if (this->params.travel.enabled)
	{
		// Get the data target.
		int currTargetIndex = this->dataSetBoss.getTargetIndex();
		glm::vec3 targetPos;
		const auto camPos = this->camera.getPosition();
		if (glm::length(camPos) > this->params.travel.camCutoff &&
			(this->prevTargetIndex == currTargetIndex ||
			 this->travelLog.find(currTargetIndex) == this->travelLog.end()))
		{
			// Camera is far enough from the origin.
			// Target is either the same as previous frame, or it's never been visited on this travel.
			targetPos = this->dataSetBoss.getTargetPosition() * this->params.worldScale.get();
			this->prevTargetIndex = currTargetIndex;
			this->travelLog.insert(currTargetIndex);
		}
		else
		{
			// Using default target at origin.
			this->prevTargetIndex = -1;
		}
		
		const auto toTarget = targetPos - camPos;
		const auto lerpedDir = glm::mix(this->camera.getLookAtDir(), glm::normalize(toTarget), this->params.travel.lookAtLerp.get());
		const auto targetDist = glm::length(toTarget);

		// This always has the camera looking at the heading direction.
		//const auto lerpedDist = targetDist * (1.0f - this->sharedParams.target.travelSpeed.get());
		//this->camera.setTarget(camPos + lerpedDir * targetDist);
		//this->camera.setDistance(lerpedDist);
		
		// This looks nicer because you're not always looking at the target, more drifty.
		this->camera.lookAt(camPos + lerpedDir * targetDist);
		const auto lerpPos = glm::mix(camPos, targetPos, this->params.travel.moveLerp.get());
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

			auto worldTransform = glm::scale(glm::mat4(1.0f), glm::vec3(this->params.worldScale));

			// Draw the data set.
			{
				//glEnable(GL_BLEND);
				//glBlendFunc(GL_ONE, GL_ONE);
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				// Draw all the points.
				ofSetColor(ofColor::white);

				this->spriteShader.begin();
				this->spriteShader.setUniform1f("uPointSize", this->sharedParams.point.size);
				this->spriteShader.setUniform1f("uAttenuation", this->sharedParams.point.attenuation);
				this->spriteShader.setUniformMatrix4f("uTransform", worldTransform);
				this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
				ofEnablePointSprites();
				{
					this->spriteShader.setUniform1f("uMaxSize", this->dataSetBoss.parameters.renderModels ? this->sharedParams.model.clipSize : std::numeric_limits<float>::max());
					this->dataSetBoss.drawPoints(this->spriteShader);

					this->spriteShader.setUniform1f("uMaxSize", this->dataSetDes.parameters.renderModels ? this->sharedParams.model.clipSize : std::numeric_limits<float>::max());
					this->dataSetDes.drawPoints(this->spriteShader);

					this->spriteShader.setUniform1f("uMaxSize", std::numeric_limits<float>::max());
					this->dataSetVizir.drawPoints(this->spriteShader);
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
					this->dataSetBoss.drawModels(this->modelShader, worldTransform, this->scaledMesh, this->camera, this->sharedParams);
					this->dataSetDes.drawModels(this->modelShader, worldTransform, this->scaledMesh, this->camera, this->sharedParams);
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
