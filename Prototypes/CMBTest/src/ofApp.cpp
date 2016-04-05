#include "ofApp.h"
#include "GLError.h"

namespace ent
{
    //--------------------------------------------------------------
    void ofApp::setup()
    {
        ofSetLogLevel(OF_LOG_VERBOSE);
        //ofSetVerticalSync(false);

        m_tintColor = ofColor::white;

#ifdef COMPUTE_GL_2D
		m_cmbScene.setDimensions(ofDefaultVec2(ofGetWidth(), ofGetHeight()));
#elif defined(COMPUTE_GL_3D)
		m_cmbScene.setDimensions(128);
#elif defined(COMPUTE_CL_2D)
		m_cmbScene.setDimensions(ofDefaultVec2(ofGetWidth(), ofGetHeight()));
#elif defined(COMPUTE_CL_3D)
		m_cmbScene.setDimensions(256);
#endif
		m_cmbScene.setup();

#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
		m_camera.setDistance(1000);
#endif

        m_bGuiVisible = true;
    }

    //--------------------------------------------------------------
	void ofApp::update()
	{
		m_cmbScene.update();
    }

    //--------------------------------------------------------------
    void ofApp::imGui()
    {
        static const int kGuiMargin = 10;

        m_gui.begin();
        {
            ofVec2f windowPos(kGuiMargin, kGuiMargin);
            ofVec2f windowSize = ofVec2f::zero();

            ImGui::SetNextWindowPos(windowPos, ImGuiSetCond_Appearing);
            ImGui::SetNextWindowSize(ofVec2f(380, 94), ImGuiSetCond_Appearing);
            if (ImGui::Begin("CMB", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("%.1f FPS (%.3f ms/frame)", ofGetFrameRate(), 1000.0f / ImGui::GetIO().Framerate);

                ImGui::Checkbox("Restart", &m_cmbScene.m_bRestart);

                ImGui::ColorEdit3("Tint Color", &m_tintColor[0]);
                ImGui::ColorEdit3("Drop Color", &m_cmbScene.m_dropColor[0]);

                ImGui::Checkbox("Enable Dropping", &m_cmbScene.m_bDropping);
                ImGui::SliderInt("Drop Rate", &m_cmbScene.m_dropRate, 1, 60);
                ImGui::SliderFloat("Damping", &m_cmbScene.m_damping, 0.0f, 1.0f);
                ImGui::SliderFloat("Radius", &m_cmbScene.m_radius, 1.0f, 50.0f);
                ImGui::SliderFloat("Ring Size", &m_cmbScene.m_ringSize, 0.0f, 5.0f);

                windowSize.set(ImGui::GetWindowSize());
                ImGui::End();
            }

            ofRectangle windowBounds(windowPos, windowSize.x, windowSize.y);
            m_bMouseOverGui = windowBounds.inside(ofGetMouseX(), ofGetMouseY());
        }
        m_gui.end();
    }

    //--------------------------------------------------------------
    void ofApp::draw()
    {
        ofBackground(255);

        ofPushStyle();
		{
			ofEnableAlphaBlending();
			ofDisableDepthTest(); 
			
			ofSetColor(m_tintColor);

#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			m_camera.begin();
#endif
			m_cmbScene.draw();
#if defined(COMPUTE_GL_3D) || defined(COMPUTE_CL_3D)
			m_camera.end();
#endif
		}
        ofPopStyle();

        if (m_bGuiVisible) 
		{
            imGui();
        }
    }

    //--------------------------------------------------------------
    void ofApp::keyPressed(int key)
    {
        switch (key) 
		{
            case '`':
				m_bGuiVisible ^= 1;
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
    void ofApp::windowResized(int w, int h)
    {
        m_cmbScene.m_bRestart = true;
    }

    //--------------------------------------------------------------
    void ofApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void ofApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
