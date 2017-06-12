#include "ofApp.h"

double smoothNoise(ofFloatPixels & noise, double x, double y)
{
   //get fractional part of x and y
   double fractX = x - int(x);
   double fractY = y - int(y);

   //wrap around
   int x1 = (int(x) + noise.getWidth()) % noise.getWidth();
   int y1 = (int(y) + noise.getHeight()) % noise.getHeight();

   //neighbor values
   int x2 = (x1 + noise.getWidth() - 1) % noise.getWidth();
   int y2 = (y1 + noise.getHeight() - 1) % noise.getHeight();

   //smooth the noise with bilinear interpolation
   double value = 0.0;
   value += fractX     * fractY     * noise.getColor(y1, x1).r;
   value += (1 - fractX) * fractY     * noise.getColor(y1, x2).r;
   value += fractX     * (1 - fractY) * noise.getColor(y2, x1).r;
   value += (1 - fractX) * (1 - fractY) * noise.getColor(y2, x2).r;

   return value;
}

double turbulence(ofFloatPixels & noise, double x, double y, double size)
{
  double value = 0.0, initialSize = size;

  while(size >= 1)
  {
	value += smoothNoise(noise, x / size, y / size) * size;
	size /= 2.0;
  }

  return(128.0 * value / initialSize);
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofDisableArbTex();
	ofSetBackgroundColor(0);

	auto setFreq = [this](float & freq){
		textures.clear();
		for(size_t i = 0; i < 10; i++){
			ofFloatPixels pixels;
			auto s = 1024.;
			pixels.allocate(s, s, OF_PIXELS_GRAY_ALPHA);
			pixels.set(0);
			for(auto l: pixels.getLines()){
				auto j = 0;
				for(auto p: l.getPixels()){
					auto f =
					        ofNoise(j / s * frequency0 + i * s, l.getLineNum() / s * frequency0 + i * s)/4. +
					        ofNoise(j / s * frequency1 + i * s, l.getLineNum() / s * frequency1 + i * s)/4. +
					        ofNoise(j / s * frequency2 + i * s, l.getLineNum() / s * frequency2 + i * s)/4. +
					        ofNoise(j / s * frequency3 + i * s, l.getLineNum() / s * frequency3 + i * s)/4.
					        ;
					//f = ofMap(f, colorramp_low, colorramp_high, 0, 1, true);
					p[0] = f;
					p[1] = f;
					j+=1;
				}
			}
			textures.emplace_back();
			textures.back().allocate(pixels);
		}
	};

	frequency0.ownListener(setFreq);
	frequency1.ownListener(setFreq);
	frequency2.ownListener(setFreq);
	frequency3.ownListener(setFreq);
//	colorramp_high.ownListener(setFreq);
//	colorramp_low.ownListener(setFreq);

	gui.getFloatSlider("frequency0").setUpdateOnReleaseOnly(true);
	gui.getFloatSlider("frequency1").setUpdateOnReleaseOnly(true);
	gui.getFloatSlider("frequency2").setUpdateOnReleaseOnly(true);
	gui.getFloatSlider("frequency3").setUpdateOnReleaseOnly(true);
//	gui.getFloatSlider("color ramp low").setUpdateOnReleaseOnly(true);
//	gui.getFloatSlider("color ramp high").setUpdateOnReleaseOnly(true);

	nebulaShader.load("nebula.vert", "nebula.frag");

	float f;
	setFreq(f);
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	//textures[0].draw(0,0);
	//ofEnableBlendMode(OF_BLENDMODE_ADD);
	nebulaShader.begin();
	nebulaShader.setUniform1f("colorramp_low", colorramp_low);
	nebulaShader.setUniform1f("colorramp_high", colorramp_high);
	camera.begin();
	auto i = 0;
	for(auto & t: textures){
		auto z = -(i+1)*textureSeparation*t.getWidth();
		if(i>0){
			auto nextT = glm::vec3(0,0,z);
			auto distance = glm::distance2(camera.getPosition(), nextT); // (textureSeparation*t.getWidth()*10*textureSeparation*t.getWidth()*10);
			float alpha = ofMap(distance, 0, pow(textureSeparation*t.getWidth()*textures.size(),2), 0.8, 0, true);
			ofSetColor(255,alpha*255.);
		}else{
			auto nextT = glm::vec3(0,0,z);
			auto alpha = glm::distance2(camera.getPosition(), nextT) / (textureSeparation*t.getWidth()*textureSeparation*t.getWidth());
			alpha = ofMap(alpha, 0, 1, 0.1, 0.8);
			ofSetColor(255,alpha*255.);
		}
		nebulaShader.setUniformTexture("texture0", t, 0);
		t.draw(-t.getWidth()/2, -t.getWidth()/2, z);
		i++;
	}
	camera.end();
	nebulaShader.end();

	gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if(key==OF_KEY_UP){
		camera.move(0,0,-10);
	}
	auto t = textures.front();
	if(camera.getPosition().z < -textureSeparation*t.getWidth()){
		textures.erase(textures.begin());
		textures.push_back(t);
		auto p = camera.getPosition();
		camera.setPosition(p.x, p.y, p.z + textureSeparation*t.getWidth());
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
