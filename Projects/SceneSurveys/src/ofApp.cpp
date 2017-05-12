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

	// we want each box to have a different color so let's add
	// as many colors as boxes
	//this->galaxyMesh.getColors().resize(matrices.size());
	//for (size_t i = 0; i<this->galaxyMesh.getColors().size(); i++) {
	//	this->galaxyMesh.getColors()[i] = ofColor::fromHsb(i % 255, 255, 255);
	//}

	//// then we tell the vbo that colors should be used per instance by using
	//// ofVbo::setAttributeDivisor
	//this->galaxyMesh.getVbo().setAttributeDivisor(ofShader::COLOR_ATTRIBUTE, 1);

	// Load the shaders.
	this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
	this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::Mass, "mass");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
	this->spriteShader.bindDefaults();
	this->spriteShader.linkProgram();

	this->modelSettings = ofShader::Settings();
	this->modelSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/instanced.vert";
	this->modelSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/instanced.frag";
	this->modelSettings.bindDefaults = true;
	this->modelShader.setup(this->modelSettings);

	// Setup the camera.
	this->eventListeners.push_back(this->nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->eventListeners.push_back(this->farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
		this->sharedParams.model.clipDistance.setMax(val);
	}));

	// Setup renderer and post effects using resize callback.
	this->windowResized(ofGetWidth(), ofGetHeight());

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("Fira Code", 11, true, true, 72);
	this->gui.setup("Surveys", "settings.json");
	this->gui.add(this->parameters);
	this->gui.add(this->sharedParams);
	this->gui.add(this->dataSetBoss.parameters);
	this->gui.add(this->dataSetDes.parameters);
	this->gui.add(this->dataSetVizir.parameters);
	this->gui.add(this->renderer.parameters);
	this->gui.add(this->postParams);
	this->gui.minimizeAll();

	this->timeline.setup();
	this->timeline.setDefaultFontPath("Fira Code");
	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
	this->timeline.setDurationInSeconds(60 * 5);
	this->timeline.setAutosave(false);

	this->gui.setTimeline(&this->timeline);
	this->gui.loadFromFile("settings.json");
}

//--------------------------------------------------------------
void ofApp::exit()
{

}

//--------------------------------------------------------------
void ofApp::update()
{
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

			auto worldTransform = glm::scale(glm::mat4(1.0f), glm::vec3(this->worldScale));

			// Draw the data set.
			{
				//glEnable(GL_BLEND);
				//glBlendFunc(GL_ONE, GL_ONE);
				ofEnableBlendMode(OF_BLENDMODE_ADD);
				ofDisableDepthTest();

				// Draw all the points.
				this->spriteShader.begin();
				this->spriteShader.setUniform1f("uPointSize", this->sharedParams.point.size);
				this->spriteShader.setUniform1f("uAttenuation", this->sharedParams.point.attenuation);
				this->spriteShader.setUniform1f("uMaxSize", this->sharedParams.model.clipSize);
				this->spriteShader.setUniformMatrix4f("uTransform", worldTransform);
				this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
				ofEnablePointSprites();
				{
					this->dataSetBoss.drawPoints(this->spriteShader);
					this->dataSetDes.drawPoints(this->spriteShader);
					this->dataSetVizir.drawPoints(this->spriteShader);
				}
				ofDisablePointSprites();
				this->spriteShader.end();

				// Draw the models for galaxies near the camera.
				this->modelShader.begin();
				{
					this->dataSetBoss.drawModels(this->modelShader, worldTransform, this->scaledMesh, this->camera, this->sharedParams);
					this->dataSetDes.drawModels(this->modelShader, worldTransform, this->scaledMesh, this->camera, this->sharedParams);
				}
				this->modelShader.end();

				ofEnableBlendMode(OF_BLENDMODE_ALPHA);

				ofNoFill();
				ofDrawBox(1);
				ofFill();
			}

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
void ofApp::keyPressed(int key){

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
