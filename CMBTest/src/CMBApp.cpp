#include "CMBApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void CMBApp::setup()
    {
        paramGroup.setName("RIPPLES");
        paramGroup.add(tintColor.set("TINT COLOR", ofColor::white, ofColor::black, ofColor::white));
        paramGroup.add(dropColor.set("DROP COLOR", ofColor::white, ofColor::black, ofColor::white));
        paramGroup.add(bDropOnPress.set("DROP ON PRESS", false));
        paramGroup.add(bDropUnderMouse.set("DROP UNDER MOUSE", false));
        paramGroup.add(dropRate.set("DROP RATE", 1, 1, 30));
        paramGroup.add(damping.set("DAMPING", 0.995f, 0.0, 1.0));
        paramGroup.add(radius.set("RADIUS", 10.0, 1.0, 50.0));
        paramGroup.add(bRestart.set("RESTART", true));

        guiPanel.setup(paramGroup, "ripples.xml");
        guiPanel.loadFromFile("ripples.xml");

        bRestart = true;
        bGuiVisible = true;

//        shader.load("shaders/ripples");
        shader.load("", "shaders/ripples.frag");
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

        // Ignore presses over the GUI.
        bool bMousePressed = ofGetMousePressed() && (!bGuiVisible || !guiPanel.getShape().inside(ofGetMouseX(), ofGetMouseY()));

        // Add new drops.
        ofPushStyle();
        ofPushMatrix();

        srcFbo.begin();
        {
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
            guiPanel.draw();
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
