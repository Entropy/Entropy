#include "ofApp.h"

namespace{
constexpr int NUM_PARTICLES = 1000;

ofFbo::Settings fboSettings(){
	ofFbo::Settings settings;
	settings.width = ofGetWidth();
	settings.height = ofGetHeight();
	settings.numSamples = 8;
	settings.internalformat = GL_RGBA32F;
	//settings.depthStencilInternalFormat = GL_DEPTH32F_STENCIL8;
	//settings.depthStencilAsTexture = true;
	settings.useDepth = true;
	settings.useStencil = false;
	return settings;
}

void billboard(ofTrueTypeFont & f, std::string text, glm::mat4 projection, glm::mat4 modelview, glm::vec3 pos){
	auto rViewport = ofGetCurrentViewport();

	auto mat = projection * modelview;
	auto dScreen4 = mat * glm::vec4(pos,1.0);
	auto dScreen = dScreen4.xyz() / dScreen4.w;
	dScreen += glm::vec3(1.0) ;
	dScreen *= 0.5;

	dScreen.x += rViewport.x;
	dScreen.x *= rViewport.width;

	dScreen.y += rViewport.y;
	dScreen.y *= rViewport.height;

	if (dScreen.z >= 1) return;


	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofPushMatrix();
	ofLoadIdentityMatrix();

	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofPushMatrix();

	glm::mat4 modelView;
	modelView = glm::translate(modelView, glm::vec3(-1,-1,0));
	modelView = glm::scale(modelView, glm::vec3(2/rViewport.width, 2/rViewport.height, 1));
	modelView = glm::translate(modelView, glm::vec3(dScreen.x, dScreen.y, 0));
	ofLoadMatrix(modelView);
	auto m = f.getStringMesh(text, 0, 0, false);
	f.getFontTexture().bind();
	m.draw();
	f.getFontTexture().unbind();


	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofPopMatrix();

	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofPopMatrix();
}
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	ofSetBackgroundColor(0);
	auto half_side = side/2;
	for(int i=0;i<NUM_PARTICLES;i++){
		positions.emplace_back(ofRandom(-half_side, half_side), ofRandom(-half_side, half_side), ofRandom(-half_side, half_side));
		types.emplace_back(ofRandom(3));
	}


	auto relDistanceChanged = [this](float & relDistance){
		relations.clear();
		auto maxDistance = (relDistance * side) * (relDistance * side);
		for(size_t i=0;i<positions.size();i++){
			for(size_t j=0;j<positions.size();j++){
				auto distance = glm::distance2(positions[i], positions[j]);
				if(distance<maxDistance){
					relations.emplace_back(i,j);
				}
			}
		}
	};

	auto d = relDistance.get();
	relDistanceChanged(d);
	listeners.push_back(relDistance.newListener(relDistanceChanged));

	fbo.allocate(fboSettings());
	fbo2.allocate(fboSettings());

	fbo.getDepthTexture().setRGToRGBASwizzles(true);

	postEffects.resize(ofGetWidth(), ofGetHeight());
	renderer.setup(side);
	renderer.resize(ofGetWidth(), ofGetHeight());

	ofTtfSettings fontSettings("Kontrapunkt Bob Light.otf", 20);
	fontSettings.dpi = 72;
	fontSettings.antialiased = true;
	font.load(fontSettings);

	ofxTextureRecorder::Settings recorderSettings(fbo.getTexture());
	recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
	recorderSettings.folderPath = "render";
	recorder.setup(recorderSettings);

	billboardShader.load("billboard.vert.glsl", "billboard.frag.glsl");

	particleTexts["H"] = font.getStringMesh("H", 0, 0);
	particleTexts["He"] = font.getStringMesh("He", 0, 0);
	particleTexts["Li"] = font.getStringMesh("Li", 0, 0);
	particleTexts["Hydrogen"] = font.getStringMesh("Hydrogen", 0, 0);
	particleTexts["Helium"] = font.getStringMesh("Helium", 0, 0);
	particleTexts["Lithium"] = font.getStringMesh("Lithium", 0, 0);
}

//--------------------------------------------------------------
void ofApp::update(){
	//cam.orbitDeg(ofGetFrameNum()/60.f,0,side*0.6);
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableDepthTest();
	fbo.begin();
	ofClear(0,255);
	cam.begin();
	auto maxScreenDistance = (maxDistance * side) * (maxDistance * side);


//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);
	renderer.drawWithDOF(cam, [this, maxScreenDistance](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		ofLoadMatrix(modelview);
		for(auto & r: relations){
			auto p1 = positions[r.first];
			auto p2 = positions[r.second];
			auto distance = glm::distance2(cam.getPosition(), p1);
			auto pct = 1-ofClamp(distance / maxScreenDistance, 0, 1);
			ofSetColor(pct*255, accumValue*255);
			ofDrawLine(p1,p2);
		}
	});

	renderer.drawWithDOF(cam, [this, maxScreenDistance](float accumValue, glm::mat4 projection, glm::mat4 modelview){
		auto i = 0;
		billboardShader.begin();
		auto viewport = ofGetCurrentViewport();
		auto viewportv4 = glm::vec4(viewport.position.xy(), viewport.width, viewport.height);
		billboardShader.setUniform4f("viewport", viewportv4);
		billboardShader.setUniformMatrix4f("projectionMatrix", projection);
		billboardShader.setUniformMatrix4f("modelViewMatrix", modelview);
		billboardShader.setUniformMatrix4f("modelViewProjectionMatrix", projection * modelview);
		billboardShader.setUniformTexture("tex0", font.getFontTexture(), 0);
		for(auto & p: positions){
			auto distance = glm::distance2(cam.getPosition(), p);
			auto pctDistance = ofClamp(distance / maxScreenDistance, 0, 1);
			auto pctColor = 1-pctDistance;
			billboardShader.setUniform1f("pctColor", pctColor);
			billboardShader.setUniform1f("accumValue", accumValue);
			billboardShader.setUniform4f("billboard_position", glm::vec4(p, 1.0));
			std::string text = "";
			if(pctDistance>fulltextDistance){
				switch(types[i++]){
					case 0:
						text = "H";
					break;
					case 1:
						text = "He";
					break;
					case 2:
						text = "Li";
					break;
				}
			}else{
				switch(types[i++]){
					case 0:
						text = "Hydrogen";
					break;
					case 1:
						text = "Helium";
					break;
					case 2:
						text = "Lithium";
					break;
				}
			}
			particleTexts[text].draw();
			//font.getStringMesh(text, 0, 0).draw();
			//billboard(font, text, cam.getProjectionMatrix(), modelview, p);
		}
		billboardShader.end();
	});
	cam.end();
	fbo.end();
	postEffects.process(fbo.getTexture(), fbo2, postParameters);

	if(save){
		recorder.save(fbo2.getTexture());
	}

	ofDisableDepthTest();
//	ofDisableAlphaBlending();
	fbo2.getTexture().draw(0,0);
	//fbo.getDepthTexture().draw(0,0);
	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key=='f'){
		ofToggleFullscreen();
		fbo.allocate(fboSettings());
		fbo2.allocate(fboSettings());
		postEffects.resize(ofGetWidth(), ofGetHeight());
	}
	if(key=='s'){
		save = !save;
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
