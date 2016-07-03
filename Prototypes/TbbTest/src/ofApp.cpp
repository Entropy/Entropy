#include "ofApp.h"
#include "ofxObjLoader.h"

//#define _TEAPOT

//--------------------------------------------------------------
void ofApp::setup()
{
    //ofSetFrameRate(60);
    ofSetVerticalSync(false);
    
    ofBackground(0);
    nm::Octree<ofVec3f>::setMaxDepth(5);
    particleSystem.init(ofVec3f(-400.f), ofVec3f(400.f));
    
#ifdef _TEAPOT
    ofVboMesh mesh;
    ofxObjLoader::load("teapot.obj", mesh);
    for (auto& v : mesh.getVertices())
    {
        v *= 80.f;
        v.y -= 80.f;
        particleSystem.addParticle(v);
    }
#else
    for (unsigned i = 0; i < nm::ParticleSystem::MAX_PARTICLES; ++i)
    {
        ofVec3f v(400.f * ofSignedNoise(i / 2000.f, 10),
                   400.f * ofSignedNoise(i / 2000.f, 1e-6),
                   400.f * ofSignedNoise(i / 2000.f, 1e6));
        particleSystem.addParticle(v);
    }
#endif
    
    gui.setup();
    for (unsigned i = 0; i < nm::ParticleSystem::NUM_LIGHTS; ++i)
    {
        particleSystem.lightIntensities[i] = 1.f;
        particleSystem.lightRadiuses[i] = 1.f;
        particleSystem.lightCols[i].set(1.f, 1.f, 1.f, 1.f);
        
        string iStr = ofToString(i);
        persistent.add("lightPosns" + iStr, particleSystem.lightPosns[i], ofVec3f(-2000.f), ofVec3f(2000.f));
        persistent.add("lightIntensities" + iStr, particleSystem.lightIntensities[i], 0.f, 5.f);
        persistent.add("lightRadiuses" + iStr, particleSystem.lightRadiuses[i], 0.f, 4000.f);
        persistent.add("lightCols" + iStr, particleSystem.lightCols[i], ofFloatColor(0.f), ofFloatColor(1.f));
    }
    persistent.add("roughness", particleSystem.roughness, 0.f, 1.f);
    persistent.load("settings/settings.xml");
}

//--------------------------------------------------------------
void ofApp::update()
{
    particleSystem.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofSetWindowTitle(ofToString(ofGetFrameRate(), 2));
    
    cam.begin();
    particleSystem.draw();
    cam.end();
    
    drawGui();
}

void ofApp::exit()
{
    persistent.save("settings/settings.xml");
}

void ofApp::drawGui()
{
    gui.begin();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    guiFromPersistent(persistent);
    gui.end();
}

void ofApp::guiFromPersistent(ofxPersistent& persistent)
{
    for (auto& pair : persistent.getFloats())
    {
        ImGui::SliderFloat(pair.first.c_str(), pair.second.getValue(), pair.second.getMin(), pair.second.getMax());
    }
    for (auto& pair : persistent.getVec2fs())
    {
        ImGui::SliderFloat2(pair.first.c_str(), &pair.second.getValue()->x, pair.second.getMin().x, pair.second.getMax().x);
    }
    for (auto& pair : persistent.getVec3fs())
    {
        ImGui::SliderFloat3(pair.first.c_str(), &pair.second.getValue()->x, pair.second.getMin().x, pair.second.getMax().x);
    }
    for (auto& pair : persistent.getBools())
    {
        ImGui::Checkbox(pair.first.c_str(), pair.second.getValue());
    }
    for (auto& pair : persistent.getFloatColors())
    {
        ImGui::ColorEdit4(pair.first.c_str(), &pair.second.getValue()->r);
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

}
