#include "ofApp.h"
#include "lb/gl/GLError.h"
#include "glm/glm.hpp"
#include "lb/math/MatrixTools.h"

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup()
{
    lb::CheckGLError(); // clear GLEW errors

    // Set up cameras
    m_camera.setupPerspective( false, 45.0f, 1.0f, 1000.0f );
    m_debugCamera.setupPerspective( false, 60.0f, 1.0f, 10000.0f );

    m_camera.setAspectRatio( ( ofGetWindowWidth() ) / ofGetWindowHeight() );
    m_debugCamera.setAspectRatio( ( ofGetWindowWidth() ) / ofGetWindowHeight() );

    m_camera.setPosition( 0.0, 0.0, 600.0 );
    
    m_camera.setAutoDistance( false );
    m_camera.setDistance( 500 );

    // Load Shader
    m_shader.load( "shaders/passthrough.vert", "shaders/clustered_lights.frag" );
    m_shader.printActiveUniforms();
    m_shader.printActiveUniformBlocks();

    // Set up view ubo
    const int viewUboBinding = 1;
    m_viewUbo.Init( viewUboBinding );
    m_viewUbo.ConfigureShader( m_shader );

    // Set up lighting
    SetupLighting();
    CreateRandomLights();

    m_sphere = ofSpherePrimitive( 1.0f, 8 );

    SetAppMode( AppMode::NORMAL_VIEW );
}

void ofApp::SetupLighting()
{  
    m_lightSystem.Init( m_camera );
    m_lightSystem.ConfigureShader( m_shader );
}

void ofApp::CreateRandomLights()
{
    // create some random lights
    float positionDist = 190.0f;
    float radius = 100.0f;

    std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
    for ( int i = 0; i < 60; ++i )
    {
        glm::vec3 offset = glm::normalize( vec3( ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ) ) ) * positionDist;
        lb::PointLight l( offset, vec3( 1.0f, 0.0f, 0.0f ), radius, i * 20.0f + 80.0f );
        l.color = vec4( glm::normalize( vec3( ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ) ) ), 1.0f );
        pointLights.push_back( l );
    }
}

void ofApp::AnimateLights()
{
    std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
    for ( int idx = 0; idx < pointLights.size(); ++idx )
    {
        lb::PointLight& light = pointLights[ idx ];
        light.intensity = ( sinf( ( ofGetElapsedTimeMillis() + idx * 20 ) / 100.0f ) * 0.5f + 0.5f ) * 1000.0f; 
    }
}

void ofApp::SetAppMode( const AppMode _mode )
{
    switch ( _mode )
    {
        case AppMode::NORMAL_VIEW:
        {
            m_camera.enableMouseInput();
            m_debugCamera.disableMouseInput();
        }
        break;

        case AppMode::DEBUG_VIEW:
        {
            m_camera.disableMouseInput();
            m_debugCamera.enableMouseInput();
        }
    }

    m_appMode = _mode;
}

void ofApp::DrawScene()
{
    // Draw big test sphere for something to shade
    ofPushMatrix();
        ofScale( 175.0f );
        m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
        m_sphere.draw();
    ofPopMatrix();
 
    ofPushMatrix();
    ofTranslate( -300.0f, 0, 0 );
        ofScale( 175.0f );
        m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
        m_sphere.draw();
    ofPopMatrix();

    ofPushMatrix();
    ofTranslate( 000.0f, -300, 0 );
        ofScale( 175.0f );
        m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
        m_sphere.draw();
    ofPopMatrix();

    ofPushMatrix();
    ofTranslate( 000.0f, 300, 0 );
        ofScale( 175.0f );
        m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
        m_sphere.draw();
    ofPopMatrix();

    ofPushMatrix();
    ofTranslate( 300.0f, 0, 0 );
        ofScale( 175.0f );
        m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
        m_sphere.draw();
    ofPopMatrix();
 }

//--------------------------------------------------------------
void ofApp::update()
{
    AnimateLights();

    m_viewUbo.Update( m_camera );
    m_lightSystem.Update( m_camera );
}

//--------------------------------------------------------------
void ofApp::draw()
{
    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );

    ofEnableDepthTest();
    ofClear( ofFloatColor( 0.1f, 0.1f, 0.1f, 1.0f ) );

    m_lightSystem.Bind();
    m_viewUbo.Bind();

    switch ( m_appMode )
    {
        case AppMode::NORMAL_VIEW:
        {
            m_camera.begin();
            {
                ofSetColor( 255, 255, 255, 255 );

                m_shader.begin();
                    DrawScene();
                m_shader.end();
             }
            m_camera.end();
        }
        break;

        case AppMode::DEBUG_VIEW:
        {
            m_debugCamera.begin();
            {
                ofSetColor( 255, 255, 255, 255 );
                m_lightSystem.DebugDrawFrustum( m_camera );
                
                m_lightSystem.DebugDrawCulledPointLights();
                m_lightSystem.DebugDrawClusteredPointLights();
                m_lightSystem.DebugDrawOccupiedClusters( m_camera );
            }
            m_debugCamera.end();
        }
        break;
    }

    char info[256];
    ofSetColor( 255, 255, 255, 255 );
    sprintf( info, "Press '1' for normal view, '2' for debug view\n\nFramerate: %.0f\nVisible Lights: %u", ofGetFrameRate(), m_lightSystem.GetNumVisibleLights() );
    ofDrawBitmapString( info, 50, 50 );

    if ( ofGetFrameRate() < 59.0f ) ofLogNotice() << "Low FPS " << ofGetFrameRate() << ", frame num: " << ofGetFrameNum() << endl;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch ( key )
    {
        case '1':
            SetAppMode( AppMode::NORMAL_VIEW );
        break;

        case '2':
            SetAppMode( AppMode::DEBUG_VIEW );
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
