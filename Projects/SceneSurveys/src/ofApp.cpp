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

	auto shaderSettings = ofShader::Settings();
	shaderSettings.bindDefaults = true;
	shaderSettings.shaderFiles[GL_VERTEX_SHADER] = "shaders/galaxy.vert";
	shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = "shaders/galaxy.frag";
	this->sphereShader.setup(shaderSettings);

	// Build the texture.
	entropy::LoadTextureImage(entropy::GetSceneAssetPath("Surveys", "images/sprites.png"), this->spriteTexture);

	// Load the shader.
	this->spriteShader.setupShaderFromFile(GL_VERTEX_SHADER, "shaders/sprite.vert");
	this->spriteShader.setupShaderFromFile(GL_FRAGMENT_SHADER, "shaders/sprite.frag");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::Mass, "mass");
	this->spriteShader.bindAttribute(entropy::surveys::ExtraAttribute::StarFormationRate, "starFormationRate");
	this->spriteShader.bindDefaults();
	this->spriteShader.linkProgram();

	// Setup the camera.
	this->eventListeners.push_back(this->nearClip.newListener([this](float & val)
	{
		this->camera.setNearClip(val);
	}));
	this->eventListeners.push_back(this->farClip.newListener([this](float & val)
	{
		this->camera.setFarClip(val);
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

}

//--------------------------------------------------------------
void ofApp::draw()
{
	// Draw the scene.
	this->camera.begin();
	{
		// Draw the data set.
		ofPushMatrix();
		ofScale(this->scale);
		{
			ofEnableBlendMode(OF_BLENDMODE_ADD);
			ofDisableDepthTest();

			this->spriteShader.begin();
			this->spriteShader.setUniformTexture("uTex0", this->spriteTexture, 1);
			this->spriteShader.setUniform1f("uPointSize", this->pointSize);
			ofEnablePointSprites();
			{
				if (this->renderBoss)
				{
					this->dataSetBoss.draw(this->spriteShader);
				}
				if (this->renderDes)
				{
					this->dataSetDes.draw(this->spriteShader);
				}
				if (this->renderVizir)
				{
					this->dataSetVizir.draw(this->spriteShader);
				}
			}
			ofDisablePointSprites();
			this->spriteShader.end();

			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		}
		ofPopMatrix();

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
