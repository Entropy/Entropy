#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
    ofBackground(ofColor::black);

    cellRenderer.setup();

    bGuiVisible = true;
}


//--------------------------------------------------------------
void ofApp::imGui()
{
    static const int kGuiMargin = 10;

    gui.begin();
    {
        ofVec2f windowPos(kGuiMargin, kGuiMargin);
        ofVec2f windowSize = ofVec2f::zero();
        bMouseOverGui = cellRenderer.imGui(windowPos, windowSize);
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::update()
{
    cellRenderer.update();

    if (bMouseOverGui) {
        cam.disableMouseInput();
    }
    else {
        cam.enableMouseInput();
    }
    bMouseOverGui = false;
}

//--------------------------------------------------------------
void ofApp::draw()
{
    cam.setNearClip(0);
    cam.setFarClip(FLT_MAX);
    cam.begin();
    {
        cellRenderer.draw();
        ofDrawAxis(20);
    }
    cam.end();

    if (bGuiVisible) {
        imGui();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    switch (key) {
        case '`':
            bGuiVisible ^= 1;
            break;

        case OF_KEY_TAB:
            ofToggleFullscreen();
            break;

        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
