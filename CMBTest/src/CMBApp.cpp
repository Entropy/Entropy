#include "CMBApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void CMBApp::setup()
    {
        bDropOnPress = false;
        dropRate = 1;
        damping = 0.995f;
        radius = 10.0f;

        minDropHue = 0;
        maxDropHue = 0;
        minDropSat = 0;
        maxDropSat = 0;
        minDropBri = 255;
        maxDropBri = 255;

//        shader.load("shaders/ripples");
        shader.load("", "shaders/ripples.frag");

        bRestart = true;
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

//        tintColor.setHsb(tintHue->getPos(), tintSat->getPos(), tintBri->getPos(), tintAlpha->getPos());
        dropColor.setHsb(ofRandom(minDropHue, maxDropHue), ofRandom(minDropSat, maxDropSat), ofRandom(minDropBri, maxDropBri));

        // Add new drops.
        ofPushStyle();
        ofPushMatrix();

        srcFbo.begin();
        {
            if ((bDropOnPress && ofGetMousePressed()) || (!bDropOnPress && ofGetFrameNum() % dropRate == 0)) {
                ofSetColor(dropColor);
                ofNoFill();
                ofDrawCircle(ofGetMouseX(), ofGetMouseY(), radius);
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
    void CMBApp::draw()
    {
        ofSetColor(255);

        ofPushStyle();
        ofSetColor(tintColor);
        ofEnableAlphaBlending();
        dstFbo.draw(0, 0);
        ofPopStyle();

        swap(srcFbo, dstFbo);
    }

//--------------------------------------------------------------
void CMBApp::keyPressed(int key){

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
void CMBApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void CMBApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void CMBApp::dragEvent(ofDragInfo dragInfo){ 

}
}
