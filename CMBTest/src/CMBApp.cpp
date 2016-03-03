#include "CMBApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void CMBApp::setup()
    {
        tintColor = ofColor::white;
        dropColor = ofColor::white;

        bDropOnPress = false;
        bDropUnderMouse = false;
        dropRate = 1;

        damping = 0.995f;
        radius = 10.0f;

        bRestart = true;
        bGuiVisible = true;

        shader.load("shaders/ripples");
    }

    //--------------------------------------------------------------
    void CMBApp::restart()
    {
        float width = ofGetWidth();
        float height = ofGetHeight();

        // Allocate the FBOs.
        ofFbo::Settings fboSettings;
        fboSettings.width = width;
        fboSettings.height = height;
        fboSettings.internalformat = GL_RGBA32F;

        srcFbo.allocate(fboSettings);
        srcFbo.begin();
        {
            ofClear(0, 0);
        }
        srcFbo.end();

        dstFbo.allocate(fboSettings);
        dstFbo.begin();
        {
            ofClear(0, 0);
        }
        dstFbo.end();

        // Build a mesh to render a quad.
        mesh.clear();
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
        mesh.addVertex(ofVec3f(0, 0));
        mesh.addVertex(ofVec3f(width, 0));
        mesh.addVertex(ofVec3f(width, height));
        mesh.addVertex(ofVec3f(0, height));
        mesh.addTexCoord(ofVec2f(0, 0));
        mesh.addTexCoord(ofVec2f(width, 0));
        mesh.addTexCoord(ofVec2f(width, height));
        mesh.addTexCoord(ofVec2f(0, height));
        
        bRestart = false;
    }

    //--------------------------------------------------------------
    void CMBApp::update()
    {
        if (bRestart || srcFbo.getWidth() != ofGetWidth() || srcFbo.getHeight() != ofGetHeight()) {
            restart();
        }

        // Add new drops.
        ofPushStyle();
        ofPushMatrix();

        srcFbo.begin();
        {
            bool bMousePressed = ofGetMousePressed() && !bMouseOverGui;
            if ((bDropOnPress && bMousePressed) || (!bDropOnPress && ofGetFrameNum() % dropRate == 0)) {
                ofSetColor(dropColor);
                ofNoFill();
                if (bDropUnderMouse) {
                    ofDrawCircle(ofGetMouseX(), ofGetMouseY(), radius);
                }
                else {
                    ofDrawCircle(ofRandomWidth(), ofRandomHeight(), radius);
                }
            }
        }
        srcFbo.end();

        ofPopMatrix();
        ofPopStyle();

        // Layer the drops.
        dstFbo.begin();
        shader.begin();
        shader.setUniformTexture("uPrevBuffer", dstFbo, 1);
        shader.setUniformTexture("uCurrBuffer", srcFbo, 2);
        shader.setUniform1f("uDamping", damping / 10.0f + 0.9f);  // 0.9 - 1.0 range
        {
            mesh.draw();
        }
        shader.end();
        dstFbo.end();
    }

    //--------------------------------------------------------------
    void CMBApp::imGui()
    {
        static const int kGuiMargin = 10;

        gui.begin();
        {
            ofVec2f windowPos(kGuiMargin, kGuiMargin);
            ofVec2f windowSize = ofVec2f::zero();

            ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
            ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
            if (ImGui::Begin("CMB", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

                ImGui::Checkbox("Restart", &bRestart);

                ImGui::ColorEdit3("Tint Color", &tintColor[0]);
                ImGui::ColorEdit3("Drop Color", &tintColor[0]);

                ImGui::Checkbox("Drop On Press", &bDropOnPress);
                ImGui::Checkbox("Drop Under Mouse", &bDropUnderMouse);

                ImGui::SliderInt("Drop Rate", &dropRate, 1, 60);
                ImGui::SliderFloat("Damping", &damping, 0.0f, 1.0f);
                ImGui::SliderFloat("Radius", &radius, 1.0f, 50.0f);

                windowSize.set(ImGui::GetWindowSize());
                ImGui::End();
            }

            ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
            bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());
        }
        gui.end();
    }

    //--------------------------------------------------------------
    void CMBApp::draw()
    {
        ofSetColor(255);

        ofPushStyle();
        ofSetColor(tintColor);
        ofEnableAlphaBlending();
        dstFbo.draw(0, 0);
        ofPopStyle();

        swap(srcFbo, dstFbo);

        if (bGuiVisible) {
            imGui();
        }
    }

    //--------------------------------------------------------------
    void CMBApp::keyPressed(int key)
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
    void CMBApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void CMBApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void CMBApp::windowResized(int w, int h)
    {
        bRestart = true;
    }

    //--------------------------------------------------------------
    void CMBApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void CMBApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
