#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_NOTICE);
    ofDisableArbTex();
//    ofSetDataPathRoot("../Resources/data/");
    ofBackground(ofColor::black);

    scale = 1024.0;

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

        ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
        ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
        if (ImGui::Begin("App", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

            if (ImGui::CollapsingHeader("World", nullptr, true, true)) {
                ImGui::SliderFloat("Scale", &scale, 1.0f, 2048.0f);
            }

            windowSize.set(ImGui::GetWindowSize());
            ImGui::End();
        }

        ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
        bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());

        windowPos.y += windowSize.y + kGuiMargin;
        bMouseOverGui |= cellRenderer.imGui(windowPos, windowSize);

        windowPos.y += windowSize.y + kGuiMargin;
        bMouseOverGui |= volumeRenderer.imGui(windowPos, windowSize);
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
        cellRenderer.draw(scale);
        volumeRenderer.draw(scale);

        ofNoFill();
        ofDrawBox(0, 0, 0, scale, scale, scale);
        ofFill();
        
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
