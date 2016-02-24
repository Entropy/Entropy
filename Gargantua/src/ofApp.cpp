#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofDisableArbTex();
    noiseTex.getTexture().enableMipmap();
    noiseTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    noiseTex.load("textures/noise.png");
//    noiseTex.getTexture().generateMipmap();
//    noiseTex.getTexture().setTextureMinMagFilter(GL_MIPMAP, GL_MIPMAP);
    spaceTex.getTexture().enableMipmap();
    spaceTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    spaceTex.load("textures/space.jpg");
//    spaceTex.getTexture().generateMipmap();
//    spaceTex.getTexture().setTextureMinMagFilter(GL_MIPMAP, GL_MIPMAP);

    renderShader.load("shaders/render");

    plane.set(ofGetWidth(), ofGetHeight(), 24, 16);
    plane.mapTexCoords(0, 0, ofGetWidth(), ofGetHeight());

    mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);

    mesh.addVertex(ofVec3f(0, 0));
    mesh.addTexCoord(ofVec2f(0, 0));

    mesh.addVertex(ofVec3f(ofGetWidth(), 0));
    mesh.addTexCoord(ofVec2f(ofGetWidth(), 0));

    mesh.addVertex(ofVec3f(0, ofGetHeight()));
    mesh.addTexCoord(ofVec2f(0, ofGetHeight()));

    mesh.addVertex(ofVec3f(ofGetWidth(), ofGetHeight()));
    mesh.addTexCoord(ofVec2f(ofGetWidth(), ofGetHeight()));

    gargantua.position = ofVec3f(0.0, 0.0, -8.0);
    gargantua.radius = 0.1f;
    gargantua.ringRadiusInner = 0.9f;
    gargantua.ringRadiusOuter = 6.0f;
    gargantua.ringThickness = 0.15f;
    gargantua.mass = 1000.0f;

    bGuiVisible = true;

    camera.setDistance(8.0);
    cameraFOV = 50;
    cameraSensitivity = 0.4;
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
        if (ImGui::Begin("Gargantua", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

            if (ImGui::CollapsingHeader("Camera", nullptr, true, true)) {
                ImGui::SliderFloat("FOV", &cameraFOV, 0, 180);
                ImGui::SliderFloat("Sensitivity", &cameraSensitivity, 0.0, 1.0);
            }

            if (ImGui::CollapsingHeader("Black Hole", nullptr, true, true)) {
                ImGui::SliderFloat3("Position", &gargantua.position[0], -1.0, 1.0);
                ImGui::SliderFloat("Radius", &gargantua.radius, 0.0, 1.0);
                ImGui::SliderFloat("Mass", &gargantua.mass, 0.0, 1000.0);

                ImGui::SetNextTreeNodeOpened(true, ImGuiSetCond_Appearing);
                if (ImGui::TreeNode("Ring")) {
                    ImGui::SliderFloat("Inner Radius", &gargantua.ringRadiusInner, 0.0, gargantua.ringRadiusOuter);
                    ImGui::SliderFloat("Outer Radius", &gargantua.ringRadiusOuter, 0.0, 10.0);
                    ImGui::SliderFloat("Thickness", &gargantua.ringThickness, 0.0, 1.0);

                    ImGui::TreePop();
                }
            }

            windowSize.set(ImGui::GetWindowSize());
            ImGui::End();
        }

        ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
        bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());
    }
    gui.end();
}

//--------------------------------------------------------------
void ofApp::update()
{
//    if (bMouseOverGui) {
//        cam.disableMouseInput();
//    }
//    else {
//        cam.enableMouseInput();
//    }
    bMouseOverGui = false;

    camera.setTarget(gargantua.position);
    camera.setRotationSensitivity(cameraSensitivity, cameraSensitivity, cameraSensitivity);
    camera.setFov(cameraFOV);
    camera.begin();
    {

    }
    camera.end();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    static const int kNumChannels = 2;
    renderShader.begin();

    renderShader.setUniform3f("uCamera.position", camera.getPosition());
    renderShader.setUniform1f("uCamera.fov", ofDegToRad(camera.getFov()));

    renderShader.setUniform3f("uGargantua.position", gargantua.position);
    renderShader.setUniform1f("uGargantua.radius", gargantua.radius);
    renderShader.setUniform1f("uGargantua.ringRadiusInner", gargantua.ringRadiusInner);
    renderShader.setUniform1f("uGargantua.ringRadiusOuter", gargantua.ringRadiusOuter);
    renderShader.setUniform1f("uGargantua.ringThickness", gargantua.ringThickness);
    renderShader.setUniform1f("uGargantua.mass", gargantua.mass);

    renderShader.setUniform3f("iResolution", ofGetWidth(), ofGetHeight(), 1.0);
    renderShader.setUniform1f("iGlobalTime", ofGetElapsedTimef());
    renderShader.setUniform3f("iChannelResolution[0]", ofVec3f(noiseTex.getWidth(), noiseTex.getHeight(), 1));
    renderShader.setUniform3f("iChannelResolution[1]", ofVec3f(spaceTex.getWidth(), spaceTex.getHeight(), 1));
    renderShader.setUniformTexture("iChannel0", noiseTex, 1);
    renderShader.setUniformTexture("iChannel1", spaceTex, 2);
    {
        ofPushMatrix();
        ofTranslate(plane.getWidth() * 0.5, plane.getHeight() * 0.5);
        {
            plane.draw();
        }
        ofPopMatrix();
//        mesh.draw();
    }
    renderShader.end();

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
