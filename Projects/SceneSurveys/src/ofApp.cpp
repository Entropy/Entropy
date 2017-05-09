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

	auto sphereSettings = ofShader::Settings();
	sphereSettings.bindDefaults = true;
	sphereSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/galaxy.vert";
	sphereSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/galaxy.frag";
	this->sphereShader.setup(sphereSettings);

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

	this->eventListeners.push_back(this->modelResolution.newListener([this](int & val)
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

	modelSettings = ofShader::Settings();
	modelSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/instanced.vert";
	modelSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/instanced.frag";
	modelSettings.bindDefaults = true;
	this->modelShader.setup(modelSettings);

	// Setup the camera.
	this->eventListeners.push_back(this->nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->eventListeners.push_back(this->farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
		this->modelDistance.setMax(val);
	}));

	// Setup the gui and timeline.
	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("Fira Code", 11, true, true, 72);
	this->gui.setup("Surveys", "settings.json");
	this->gui.add(this->cameraParams);
	this->gui.add(this->renderParams);
	this->gui.add(this->dataSetBoss.parameters);
	this->gui.add(this->dataSetDes.parameters);
	this->gui.add(this->dataSetVizir.parameters);
	this->gui.minimizeAll();

	this->timeline.setup();
	this->timeline.setDefaultFontPath("Fira Code");
	this->timeline.setOffset(glm::vec2(0, ofGetHeight() - this->timeline.getHeight()));
	this->timeline.setDurationInSeconds(60 * 5);
	this->timeline.setAutosave(false);

	this->gui.setTimeline(&this->timeline);
}

//--------------------------------------------------------------
void ofApp::exit()
{

}

//--------------------------------------------------------------
void ofApp::update()
{
	auto vertT = std::filesystem::last_write_time(ofToDataPath("shaders/instanced.vert"));
	auto fragT = std::filesystem::last_write_time(ofToDataPath("shaders/instanced.frag"));
	if(vertT>shaderTime || fragT>shaderTime)
	{
		shaderTime = vertT > fragT ? vertT : fragT;
		this->modelShader.setup(modelSettings);

	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw the scene.
	this->camera.begin();
	{
		auto modelTransform = glm::scale(glm::mat4(1.0f), glm::vec3(this->scale));
		
		// Draw the data set.
		{
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			ofDisableDepthTest();

			// Draw all the points.
			this->spriteShader.begin();
			this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
			this->spriteShader.setUniform1f("uPointSize", this->pointSize);
			this->spriteShader.setUniform1f("uAttenuation", this->attenuation);
			ofEnablePointSprites();
			{
				this->dataSetBoss.drawPoints(this->spriteShader, modelTransform);
				
				if (this->renderDes)
				{
					this->dataSetDes.drawPoints(this->spriteShader, modelTransform);
				}
				if (this->renderVizir)
				{
					this->dataSetVizir.drawPoints(this->spriteShader, modelTransform);
				}
			}
			ofDisablePointSprites();
			this->spriteShader.end();

			// Draw the models for galaxies near the camera.
			this->modelShader.begin();
			{
				this->dataSetBoss.drawModels(this->modelShader, modelTransform, this->scaledMesh, this->camera, this->modelDistance);
				
				//if (this->renderDes)
				//{
				//	this->dataSetDes.drawModels(this->modelShader, this->galaxyMesh, this->modelDistance);
				//}
			}
			this->modelShader.end();

			ofEnableBlendMode(OF_BLENDMODE_ALPHA);

			ofNoFill();
			ofDrawBox(1);
			ofFill();
		}

		// Draw the galaxy.
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
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
