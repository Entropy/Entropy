#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofSetDataPathRoot("../Resources/data/");

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

    renderShader.load("shaders/render.vert", "shaders/render2.frag");

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

    gargantua.position = ofVec3f(0.0, 1.0, -1.0);
    gargantua.radius = 2.7f;
    gargantua.mass = 1.2f;
    gargantua.lensing = 4.0f;
    gargantua.speed = 0.6f;
    gargantua.ringThickness = 1.5f;

    backgroundColor.set(0.0);
    foregroundColor.set(0.7, 0.6, 0.8);
    colorMix = -1.1;
    colorBlowOut = 0.5;
    ofBackground(backgroundColor);

    bGuiVisible = true;

    camera.setDistance(10.0);
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

            if (ImGui::CollapsingHeader("Colors", nullptr, true, true)) {
                if (ImGui::ColorEdit3("Background", &backgroundColor[0])) {
                    ofBackground(backgroundColor);
                }
                if (ImGui::ColorEdit3("Foreground", &foregroundColor[0])) {
                    ofSetColor(foregroundColor);
                }
                ImGui::SliderFloat("Mix", &colorMix, -5.0, 5.0);
                ImGui::SliderFloat("Blow Out", &colorBlowOut, 0.0, 5.0);
            }

            if (ImGui::CollapsingHeader("Black Hole", nullptr, true, true)) {
                ImGui::SliderFloat3("Position", &gargantua.position[0], -5.0, 5.0);
                ImGui::SliderFloat("Radius", &gargantua.radius, 0.0, 10.0);
                ImGui::SliderFloat("Mass", &gargantua.mass, 0.0, 10.0);
                ImGui::SliderFloat("Lensing", &gargantua.lensing, 0.0, 10.0);
                ImGui::SliderFloat("Speed", &gargantua.speed, 0.0, 10.0);
                ImGui::SliderFloat("Ring Thickness", &gargantua.ringThickness, 1.0, 10.0);
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
    if (bMouseOverGui) {
        camera.disableMouseInput();
    }
    else {
        camera.enableMouseInput();
    }
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
    renderShader.setUniform3f("uCamera.upDir", camera.getUpDir());
    renderShader.setUniformMatrix4f("uCamera.orientation", camera.getOrientationQuat());
    renderShader.setUniform1f("uCamera.fov", ofDegToRad(camera.getFov()));

    renderShader.setUniform1f("uColorMix", colorMix);
    renderShader.setUniform1f("uColorBlowOut", colorBlowOut);

    renderShader.setUniform3f("uBlackHole.position", gargantua.position);
    renderShader.setUniform1f("uBlackHole.radius", gargantua.radius);
    renderShader.setUniform1f("uBlackHole.ringThickness", gargantua.ringThickness);
    renderShader.setUniform1f("uBlackHole.mass", gargantua.mass);
    renderShader.setUniform1f("uBlackHole.lensing", gargantua.lensing);
    renderShader.setUniform1f("uBlackHole.speed", gargantua.speed);

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

        case 'R':
            renderShader.load("shaders/render.vert", "shaders/render2.frag");
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
