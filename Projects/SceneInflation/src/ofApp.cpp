#include "ofApp.h"

#include "Helpers.h"

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup()
{
	ofDisableArbTex();
	ofSetDataPathRoot("../../../Resources/data/entropy/scene/Inflation");
	//ofSetTimeModeFixedRate(ofGetFixedStepForFps(60));
	ofBackground(ofColor::black);

	// Marching Cubes
	gpuMarchingCubes.setup(300 * 1024 * 1024);

	// Noise Field
	noiseField.setup(gpuMarchingCubes.resolution);

	gpuMarchingCubes.resolution = 128;

	// Setup renderers.
	this->renderer.setup(1);
	this->renderer.resize(ofGetWidth(), ofGetHeight());


	// Setup big bang.
	for(size_t i=0;i<parameters.postBigBangColors.size();i++){
		parameters.postBigBangColors[i] = noiseField.octaves[i].color;
	}

	transitionParticles.setup();
	clearParticlesVel.loadCompute("shaders/compute_clear_color.glsl");

	cameraDistanceBeforeBB = 1;
	this->camera.setDistance(cameraDistanceBeforeBB);
	this->camera.setNearClip(0.01f);
	this->camera.setFarClip(6.0f);

	now = 0;
	t_bigbang = 0;
	state = PreBigBang;
	scale = 1;
	noiseField.scale = scale;
	for(size_t i=0;i<noiseField.octaves.size()/2;i++){
		noiseField.octaves[i].color = parameters.preBigbangColors[i];
	}
	resetWavelengths();
	needsParticlesUpdate = true;

	ofFbo::Settings settings;
	settings.width = ofGetWidth();
	settings.height = ofGetHeight();
	settings.internalformat = GL_RGBA32F;
	settings.numSamples = 4;

	fbo.allocate(settings);
	postFbo.allocate(settings);
	postEffects.resize(ofGetWidth(), ofGetHeight());

	ofxGuiSetDefaultWidth(250);
	ofxGuiSetFont("Fira Code", 11, true, true, 72);
	gui.setup("Inflation","settings.json");
	gui.add(noiseField.parameters);
	gui.add(gpuMarchingCubes.parameters);
	gui.add(renderer.parameters);
	gui.add(postParameters);
	gui.add(parameters);

	textField.setup(textParam);
	gui.add(&textField);
	gui.minimizeAll();

	cameraPath.arc(glm::vec3(0), 0.6, 0.5, 0, 360, true, 4800);
	for(auto & p: cameraPath){
		std::swap(p.y, p.z);
	}

	timeline.setup();
	timeline.setDefaultFontPath("Fira Code");
	timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
	timeline.setDurationInSeconds(60*5);
	timeline.setAutosave(false);

	gui.setTimeline(&timeline);

	listeners.push_back(ofEvents().mousePressed.newListener(this, &ofApp::mousePressed));

	listeners.push_back(timeline.events().pageChanged.newListener([this](ofxTLPageEventArgs&page){
		timeline.setOffset(glm::vec2(0, ofGetHeight() - timeline.getHeight()));
	}));


	listeners.push_back(parameters.render.record.newListener([this](bool & record){
		if(record){
			auto path = ofSystemLoadDialog("Record to folder:",true);
			if(path.bSuccess){
				ofxTextureRecorder::Settings recorderSettings(fbo.getTexture());
				recorderSettings.imageFormat = OF_IMAGE_FORMAT_JPEG;
				recorderSettings.folderPath = path.getPath();
				recorder.setup(recorderSettings);
			}else{
				this->parameters.render.record = false;
			}
		}
	}));
}

//--------------------------------------------------------------
void ofApp::resetWavelengths()
{
	float wl = noiseField.resolution/4;
	targetWavelengths[0] = wl;
	noiseField.octaves[0].wavelength = wl;
	noiseField.octaves[0].advanceTime = true;
	noiseField.octaves[0].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[1] = wl;
	noiseField.octaves[1].wavelength = wl;
	noiseField.octaves[1].advanceTime = true;
	noiseField.octaves[1].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[2] = wl;
	noiseField.octaves[2].wavelength = wl;
	noiseField.octaves[2].advanceTime = true;
	noiseField.octaves[2].frequencyTime = 1/wl;
	wl /= 2;
	targetWavelengths[3] = wl;
	noiseField.octaves[3].wavelength = wl;
	noiseField.octaves[3].advanceTime = true;
	noiseField.octaves[3].frequencyTime = 1/wl;
	for(size_t i=4;i<noiseField.octaves.size();i++){
		noiseField.octaves[i].enabled = false;
	}
}

void ofApp::resetWavelength(size_t octave){
	auto wl = noiseField.resolution/4;
	for(size_t i=0;i<octave;i++){
		wl /= 2;
	}
	noiseField.octaves[octave].wavelength = wl / scale;
	noiseField.octaves[octave].advanceTime = true;
}

//--------------------------------------------------------------
void ofApp::exit()
{

}

//--------------------------------------------------------------
void ofApp::update(){
	parameters.preBigbangColors[0] = parameters.colors.color1.get();
	parameters.preBigbangColors[1] = parameters.colors.color2.get();
	parameters.preBigbangColors[2] = parameters.colors.color3.get();
	parameters.preBigbangColors[3] = parameters.colors.color4.get();

	auto dt = ofGetLastFrameTime();
	if (parameters.runSimulation) {
		now += dt;
		switch(state){
			case PreBigBang:
				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
					noiseField.octaves[i].color = parameters.preBigbangColors[i];
				}
			break;
			case PreBigBangWobble:{
				t_from_bigbang = now - t_bigbang;
				float pct = t_from_bigbang/parameters.equations.preBigBangWobbleDuration;
				pct *= pct;
				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
					noiseField.octaves[i].wavelength = targetWavelengths[i] * ofMap(pct, 0, 1, 1, 0.4);
					noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * (1+pct);
					auto color = parameters.preBigbangColors[i];
					noiseField.octaves[i].color = color.lerp(parameters.postBigBangColors[i], pct * 0.5);
				}
			}break;

			case BigBang:{
				t_from_bigbang = now - t_bigbang;
				scale += dt * parameters.equations.Ht;// t_from_bigbang/parameters.bigBangDuration;
				auto pct = t_from_bigbang/parameters.equations.bigBangDuration;
				if (this->parameters.controlCamera)
				{
					camera.setDistance(ofMap(pct, 0, 1, cameraDistanceBeforeBB, 0.5));
				}
				if (t_from_bigbang > parameters.equations.bigBangDuration) {
					//resetWavelengths();
					firstCycle = true;
					state = Expansion;
				}
				for(size_t i=0;i<noiseField.octaves.size()/2;i++){
					auto color = parameters.preBigbangColors[i];
					noiseField.octaves[i].color = color.lerp(parameters.postBigBangColors[i], 0.5 + pct * 0.5);
					noiseField.octaves[i].frequencyTime = 1.f/noiseField.octaves[i].wavelength * ofMap(pct,0,1,2,1);
				}
				noiseField.octaves.back().wavelength = targetWavelengths.back() * glm::clamp(1 - scale * 2, 0.8f, 1.f);
			}break;

			case Expansion:
				t_from_bigbang = now - t_bigbang;
				scale += dt * parameters.equations.Ht;// t_from_bigbang/parameters.bigBangDuration;
				noiseField.scale = scale;
				if (this->parameters.controlCamera)
				{
					if (camera.getDistance() > 0.5f)
					{
						auto d = camera.getDistance();
						d -= dt * parameters.equations.Ht;
						camera.setDistance(d);
					}
				}
				if(!firstCycle){
					for(size_t i=0;i<noiseField.octaves.size()/2;i++){
						if(noiseField.octaves[i].advanceTime){
							noiseField.octaves[i].wavelength -= dt * parameters.equations.Ht;
							if(noiseField.octaves[i].wavelength < parameters.equations.hubbleWavelength){
								noiseField.octaves[i].advanceTime = false;
								//resetWavelength(i);
							}
						}
					}
				}
			break;
			case ExpansionTransition:{
				float t_EndIn = t_transition + parameters.equations.bbTransitionIn;
				float t_EndPlateau = t_EndIn + parameters.equations.bbTransitionPlateau;
				float t_EndOut = t_EndPlateau + parameters.equations.bbTransitionOut;

				if(now < t_EndIn){
					auto pct = ofMap(now,t_transition,t_EndIn,0,1);
					pct = sqrt(sqrt(sqrt(pct)));
					if(firstCycle){
						parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtBB * 2);
					}else{
						parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtPostBB, parameters.equations.HtBB);
					}
					scale  += dt * parameters.equations.Ht;
				}else if(now < t_EndPlateau){
					auto pct = ofMap(now,t_transition,t_EndOut,0,1);
					pct = sqrt(sqrt(sqrt(pct)));
					parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtPostBB);
					scale  += dt * parameters.equations.Ht;
				}else if(now < t_EndOut){
					auto pct = ofMap(now,t_transition,t_EndOut,0,1);
					pct = sqrt(sqrt(sqrt(pct)));
					parameters.equations.Ht = ofMap(pct, 0, 1, parameters.equations.HtBB, parameters.equations.HtPostBB);
					scale  += dt * parameters.equations.Ht;
				}
				noiseField.scale = scale;
			}break;

			case ParticlesTransition:{
				float alphaParticles = (now - t_from_particles) / parameters.equations.transitionParticlesDuration;
				alphaParticles = glm::clamp(alphaParticles, 0.f, 1.f);
				transitionParticles.color = ofFloatColor(transitionParticles.color, alphaParticles);

				float alphaBlobs = (now - t_from_particles) / parameters.equations.transitionBlobsOutDuration;
				alphaBlobs = 1.0f - glm::clamp(alphaBlobs, 0.f, 1.f);
				//noiseField.speedFactor = alphaBlobs;
				renderer.alphaFactor = alphaBlobs;
				renderer.alphaFactor = alphaBlobs;
				this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
			}break;
		}

		noiseField.update();
		if (renderer.alphaFactor > 0.001) {
			gpuMarchingCubes.update(noiseField.getTexture());
		}
		parameters.equations.geometrySize = this->gpuMarchingCubes.getNumVertices();
		if (needsParticlesUpdate) {
			this->transitionParticles.setTotalVertices(this->gpuMarchingCubes.getNumVertices());
			this->transitionParticles.update(transitionParticlesPosition, noiseField.getTexture(), now);
			needsParticlesUpdate = false;
		}

//		auto * colors = gpuMarchingCubes.getGeometry().getColorBuffer().map<ofFloatColor>(GL_WRITE_ONLY);
//		for(auto i=0;i<20;i++){
//			colors[(ofGetFrameNum() + i) % gpuMarchingCubes.getNumVertices()] = ofFloatColor(3);
//		}
//		gpuMarchingCubes.getGeometry().getColorBuffer().unmap();

		//auto distance = this->getCamera(render::Layout::Back)->getEasyCam().getDistance();
		//this->getCamera(render::Layout::Back)->getEasyCam().orbitDeg(ofGetElapsedTimef()*10.f,0,distance,glm::vec3(0,0,0));

		//transitionParticles.color = ofFloatColor(transitionParticles.color, 0.0);
	}

//	if(save){
//		static size_t idx = 0;
//		auto mesh = gpuMarchingCubes.downloadGeometry();
//		mesh.save(ofToString(idx,0,4)+".ply", true);
//		idx += 1;
//	}

	parameters.cameraDistance = camera.getDistance();
	if(rotating){
		camera.setPosition(cameraPath[ofGetFrameNum() % cameraPath.size()]);
		camera.lookAt(glm::vec3(0), glm::vec3(0,1,0));
	}
	
	if(dofTimeStart > ofTime()){
		auto now = ofGetCurrentTime();
		auto diff = std::chrono::duration<double>(now - dofTimeStart).count();
		auto d = ofxeasing::map_clamp(diff, 0, 1, dofDistanceStart, dofDistanceTarget, ofxeasing::back::easeOut);
		renderer.dofDistance = d;
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	fbo.begin();
	ofClear(0,255);
	camera.begin();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//gpuMarchingCubes.getGeometry().draw(GL_TRIANGLES, 0, gpuMarchingCubes.getNumVertices());
	renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);

//	switch (state) {
//	case PreBigBang:
//	case PreBigBangWobble:
//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//		renderer.clip = true;
//		renderer.fadeEdge0 = 0.0;
//		renderer.fadeEdge1 = 0.5;
//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
//		break;
//	case BigBang:
//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//		renderer.fadeEdge0 = 0.0;
//		renderer.fadeEdge1 = 0.5;
//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);

//		ofEnableBlendMode(OF_BLENDMODE_ADD);
//		renderer.fadeEdge0 = scale*scale;
//		renderer.fadeEdge1 = scale;
//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
//		break;
//	case Expansion:
//	case ExpansionTransition:
//		ofEnableBlendMode(OF_BLENDMODE_ADD);
//		renderer.clip = false;
//		renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
//		break;
//	case ParticlesTransition:
//		renderer.clip = false;
//		if (renderer.alphaFactor > 0.001) {
//			ofEnableBlendMode(OF_BLENDMODE_ADD);
//			renderer.draw(gpuMarchingCubes.getGeometry(), 0, gpuMarchingCubes.getNumVertices(), camera);
//		}
//		ofEnableBlendMode(OF_BLENDMODE_ALPHA);
//		auto alphaBlobs = renderer.alphaFactor;
//		renderer.alphaFactor = 1;
//		renderer.draw(transitionParticles.getVbo(), 0, transitionParticles.getNumVertices(), camera);
//		renderer.alphaFactor = alphaBlobs;
//		break;
//	}

	//ofDrawSphere(dofTarget, 0.01);
	camera.end();
	fbo.end();

	postEffects.process(fbo.getTexture(), postFbo, postParameters);

	ofDisableAlphaBlending();
	ofSetColor(255);
	postFbo.draw(0,0);

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	timeline.draw();
	gui.draw();
	ofDrawBitmapString(ofGetFrameRate(), ofGetWidth()-100, 20);

	ofSetColor(0,200);
	ofDrawRectangle(ofGetWidth() - 230, 30, 220, 220);

	ofPushMatrix();
	ofTranslate(ofGetWidth() - 120, 140);
	ofScale(100,100,0);

	ofSetColor(255);
//	ofPolyline polygon;
//	polygon.arc(glm::vec3(0), 1, 1, 0, 360, true, renderer.bokehsides);
//	polygon.close();
//	polygon.draw();

	renderer.getBokehShape().draw();
	ofPopMatrix();

	if(parameters.render.record){
		recorder.save(postFbo.getTexture());
	}

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch(key){
		case 'r':
			rotating = !rotating;
		break;
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
void ofApp::mousePressed(ofMouseEventArgs & mouse){
	if(mouse.button==OF_MOUSE_BUTTON_LEFT && mouse.hasModifier(OF_KEY_CONTROL) && mouse.hasModifier(OF_KEY_SHIFT)){
		auto geometry = gpuMarchingCubes.downloadGeometry();
		auto v = camera.worldToScreen(geometry.getVertex(0));
		auto nearest = glm::distance2(v.xy(), mouse);
		for(auto & v: geometry.getVertices()){
			auto screenv = camera.worldToScreen(v);
			auto d = glm::distance2(screenv.xy(), mouse);
			if(d<nearest){
				nearest = d;
				dofTarget = v;
			}
		}
		dofTimeStart = ofGetCurrentTime();
		dofDistanceStart = renderer.dofDistance;
		dofDistanceTarget = glm::distance(camera.getPosition(), dofTarget);
	}
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
