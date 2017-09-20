#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofDirectory("videos").create();
	HPV::InitHPVEngine();
	ofSetBackgroundColor(0);
	ofSetVerticalSync(false);
}

//--------------------------------------------------------------
void ofApp::update(){

	if(!done){
		HPV::Update();
		player.seekToFrame(currentFrame);
		fbo.begin();
		ofClear(0);
		player.draw(0, 0, player.getWidth(), player.getHeight());
		fbo.end();
		recorder->save(fbo.getTexture());
		currentFrame += 1;
		if(currentFrame == player.getTotalNumFrames()){
			recorder->stop();
			done = true;
		}
	}

	if(done && !filesToOpen.empty()){
		auto next = filesToOpen.front();
		filesToOpen.pop_front();
		if(std::filesystem::exists(next)){
			if(std::filesystem::is_directory(next)){
				ofLogError() << next << "is a folder. Folders not supported, drop only files";
			}else{
				HPV::ManagerSingleton()->closeAll();
				player.init(HPV::NewPlayer());
				player.load(next.string());
				player.play();
				player.setPaused(true);

				fbo.allocate(player.getWidth(), player.getHeight(), GL_RGB);
				fbo.begin();
				ofClear(0);
				fbo.end();


				recorder.reset(new ofxTextureRecorder());
				ofxTextureRecorder::VideoSettings settings(fbo.getTexture().getTextureData(), player.getFrameRate());
				settings.videoPath = currentVideo = "videos/" + next.filename().string() + ".mov";//
				settings.maxMemoryUsage = 8000000000; // 8Gb

				// Uncomment this for lossless h264 instead of prores
				// settings.videoCodec = "libx264";
				// crf 0 == lossless  -  crf 6 == lowest quality
				// settings.extrasettings = "-preset ultrafast -crf 0";

				settings.videoCodec = "prores";
				// v 0 == highest quality  -  v 6 == lowest quality
				settings.extrasettings = "-profile:v 0";

				//ADJUST bitrate here for better quality / smaller videos if not using extra settings
				// settings.bitrate = ...;
				recorder->setup(settings);

				currentFrame = 0;
				done = false;
			}
		}else{
			ofLogError() << next << "doesn't exist";
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	if(!done){
		std::string msg= "Converting " + ofToString(currentFrame / float(player.getTotalNumFrames()) * 100, 0, 2, '0') + "% " + currentVideo;
		ofDrawBitmapString(msg, ofGetWidth()/2 - msg.size() / 2 * 8, ofGetHeight() / 2 - 4);

		// preview
		auto ratio = player.getWidth() / player.getHeight();
		fbo.draw(0,0,ofGetWidth() / 2, ofGetWidth() / 2 / ratio);

		// files queue
		auto i = 0;
		for(auto & file: filesToOpen){
			ofDrawBitmapString(file.filename(), ofGetWidth() - 200, 20 + i*20);
			i += 1;
		}
	}else{
		std::string msg("Done, drop any hpv(s) to convert to mov");
		ofDrawBitmapString(msg, ofGetWidth()/2 - msg.size() / 2 * 8, ofGetHeight()/2 - 4);
	}
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
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
	std::transform(dragInfo.files.begin(), dragInfo.files.end(), std::back_inserter(filesToOpen), [](string & path){
		return path;
	});
}
