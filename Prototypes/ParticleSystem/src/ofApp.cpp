#include "ofApp.h"
#include "glm/glm.hpp"

#include "lb/gl/GLError.h"
#include "lb/math/MatrixTools.h"
#include "lb/camera/CameraTools.h"

using namespace glm;

//--------------------------------------------------------------
void ofApp::setup()
{
    lb::CheckGLError(); // clear GLEW errors

    m_gui.setup();
    ImGui::GetIO().MouseDrawCursor = false;

    // Set up cameras
    m_camera.setupPerspective( false, 60.0f, 0.1f, 2000.0f );
    m_debugCamera.setupPerspective( false, 60.0f, 1.0f, 10000.0f );

    m_camera.setAspectRatio( ofGetWindowWidth() / (float)ofGetWindowHeight() );
    m_debugCamera.setAspectRatio( ofGetWindowWidth() / (float)ofGetWindowHeight() );

    m_camera.setPosition( 0.0, 0.0, 600.0 );
    
    m_camera.setAutoDistance( false );
    m_camera.setDistance( 500 );

    lb::CheckGLError();

    // Load Shader
    m_shader.load( "shaders/main.vert", "shaders/main.frag" );
 //   m_shader.printActiveUniforms();
 //   m_shader.printActiveUniformBlocks();

    m_skyboxShader.load( "shaders/sky_box.vert", "shaders/sky_box.frag" );
    glGenVertexArrays( 1, &m_defaultVao );

    lb::CheckGLError();

    m_particleShader.load( "shaders/particle.vert", "shaders/main.frag" );
//    m_particleShader.printActiveUniforms();

    lb::CheckGLError();

    // Set up view ubo
    const int viewUboBinding = 1;
    m_viewUbo.Init( viewUboBinding );
    m_viewUbo.ConfigureShader( m_particleShader );
    m_viewUbo.ConfigureShader( m_shader );
    m_viewUbo.ConfigureShader( m_skyboxShader );

    lb::CheckGLError();

    m_particleSystem.init( 1600, 1600, 1600 );




    // Set up lighting
    SetupLighting();

    m_sphere = ofSpherePrimitive( 1.0f, 12 );

    m_material.SetBaseColor( ofFloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
    m_material.SetMetallic( 0.0f );
    m_material.SetRoughness( 0.0f );
    m_material.SetEmissiveColor( ofFloatColor( 1.0f, 0.4f, 0.0f, 1.0f ) );
    m_material.SetEmissiveIntensity( 0.0f );

    m_particleMaterial.SetBaseColor( ofFloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );
    m_particleMaterial.SetMetallic( 0.2f );
    m_particleMaterial.SetRoughness( 0.01f );
    m_particleMaterial.SetEmissiveColor( ofFloatColor( 1.0f, 0.4f, 0.0f, 1.0f ) );
    m_particleMaterial.SetEmissiveIntensity( 0.0f );

    float aperture = 0.5f;
    float shutterSpeed = 1.0f / 60.0f;

    m_exposure = lb::CalcEVFromCameraSettings( aperture, shutterSpeed );
    m_gamma = 2.2f;

    m_skyboxMap.LoadDDSTexture( "textures/output_skybox.dds" );
    m_irradianceMap.LoadDDSTexture( "textures/output_iem.dds" );
    m_radianceMap.LoadDDSTexture( "textures/output_pmrem.dds" );

    SetAppMode( AppMode::NORMAL_VIEW );

    glEnable( GL_TEXTURE_CUBE_MAP_SEAMLESS );


/*    m_particleSystem.addParticle( glm::vec3( 0.01f, 0.01f, 0.01f ),
                                  glm::vec3( 0.0f, 0.0f, 0.0f ),
                                  1000.0f, 10.0f );
                                  */

 //   m_particleSystem.addAttractor( glm::vec3( 0.01f, 0.01f, 0.01f ), 300.0f );
 //   m_particleSystem.addRepeller( glm::vec3( 600.0f, 0.01f, 0.01f ), 300.0f );

    for ( int i = 0; i < 20000; ++i )
    {
        float mass = ofRandom( 0.01f, 0.1f );
        float radius = ofMap( mass, 0.01f, 0.1f, 1.0f, 6.0f );

         m_particleSystem.addParticle( glm::vec3( ofRandom( -500.0f, 500.0f ), ofRandom( -500.0f, 500.0f ), ofRandom( -500.0f, 500.0f ) ),
                                       glm::vec3( ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ) ),
                                       mass, radius );
    }
}

void ofApp::SetupLighting()
{  
    m_lightSystem.Init( m_camera );
    m_lightSystem.ConfigureShader( m_particleShader );
    m_lightSystem.ConfigureShader( m_shader );
    m_lightSystem.SetAmbientIntensity( 0.02f );

    CreateRandomLights();
}

void ofApp::CreateRandomLights()
{
    // create some random lights
    float positionDist = 600;
    float radius = 300.0f;

    int numPointLights = 10;

    std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
    pointLights.clear();

    for ( int i = 0; i < numPointLights; ++i )
    {
        glm::vec3 offset = vec3( ofRandom( -positionDist, positionDist ), ofRandom( -positionDist, positionDist ), ofRandom( -positionDist, positionDist ) );
        lb::PointLight l( offset, vec3( 1.0f, 1.0f, 1.0f ), radius, 8000.0f );
        l.color = vec3( glm::normalize( vec3( ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ) ) ) );
        pointLights.push_back( l );

        m_particleSystem.addParticle( glm::vec3( l.position.x, l.position.y, l.position.z ), 
                                      glm::vec3( 0.0f, 0.0f, 0.0f ), 10.0f, 1.0f );
    }
}

void ofApp::ClearPointLights()
{
    std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
    pointLights.clear();
}

void ofApp::ClearDirectionalLights()
{
    std::vector<lb::DirectionalLight>& dirLights = m_lightSystem.GetDirectionalLights();
    dirLights.clear();
}

void ofApp::AnimateLights()
{
    std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
    for ( int idx = 0; idx < pointLights.size(); ++idx )
    {
        lb::PointLight& light = pointLights[ idx ];
        light.position.y = ( sinf( ( ofGetElapsedTimeMillis() + idx * 40 ) / 1400.0f ) * 0.5f + 0.5f ) * 100.0f; 
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

void ofApp::imGui()
{
    m_gui.begin();
    {
        ImGui::Text( "Material Properties" );
        ImGui::ColorEdit4( "Base Color", (float*)&m_material.baseColor );
        ImGui::SliderFloat( "Emissive Intensity", &m_particleMaterial.emissiveIntensity, 0.0f, 1.0f );
        ImGui::ColorEdit4( "Emissive Color", (float*)&m_material.emissiveColor );

        ImGui::Separator();
        ImGui::Text( "Camera Settings" );
        ImGui::SliderFloat( "Exposure", &m_exposure, 0.01f, 10.0f );
        ImGui::SliderFloat( "Gamma", &m_gamma, 0.01f, 10.0f );

        ImGui::BeginGroup();
        ImGui::Text( "Stats" );
        ImGui::Text( "Visible Lights: %u", m_lightSystem.GetNumVisibleLights() );
        ImGui::Text( "Culled Lights: %u", m_lightSystem.GetNumCulledPointLights() );
        ImGui::Text( "Num Affected Clusters: %u", m_lightSystem.GetNumAffectedClusters() );
        ImGui::Text( "Num Light Indices: %u", m_lightSystem.GetNumPointLightIndices() );
        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate );
        ImGui::EndGroup();

        ImGui::Separator();
        ImGui::Text( "Lights" );

        ImGui::SliderFloat( "Ambient IBL Strength", &m_lightSystem.m_ambientIntensity, 0.0f, 3.0f );

        if ( ImGui::Button( "Create Point Lights" ) )
        {
            CreateRandomLights();
        }

        ImGui::SameLine();

        if ( ImGui::Button( "Clear Point Lights" ) )
        {
            ClearPointLights();
        }


        if ( ImGui::Button( "Add Dir Light" ) )
        {
            ClearDirectionalLights();

            lb::DirectionalLight dirLight1;
            dirLight1.color = glm::vec3( 1.0f, 1.0f, 1.0f );
            dirLight1.direction = glm::normalize( glm::vec3( 1.0f, -1.0f, 0.0f ) );
            dirLight1.intensity = 1.0f;

            m_lightSystem.AddDirectionalLight( dirLight1 );
        }

        ImGui::SameLine();

        if ( ImGui::Button( "Clear Dir Light" ) )
        {
            ClearDirectionalLights();
        }

        std::vector<lb::DirectionalLight>& dirLights = m_lightSystem.GetDirectionalLights();
        if ( dirLights.size() != 0 )
        {
            ImGui::ColorEdit3( "Color", (float *)&dirLights.at(0).color );
            ImGui::SliderFloat3( "Direction", (float *)&dirLights.at(0).direction, -1.0f, 1.0f );
            ImGui::SliderFloat( "Intensity", &dirLights.at(0).intensity, 0.0f, 15.0f );
        }
    }
    m_gui.end();
}

void ofApp::DrawSkybox()
{
    glDisable( GL_CULL_FACE );
    ofDisableDepthTest();

    m_skyboxShader.begin();
        m_skyboxShader.setUniform1f( "uExposure", m_exposure );
        m_skyboxShader.setUniform1f( "uGamma", m_gamma );
        m_skyboxShader.setUniform1i( "uCubeMap", 3 );

        // draw full-screen quad
        glBindVertexArray( m_defaultVao );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
    m_skyboxShader.end();

    ofEnableDepthTest();
    glEnable( GL_CULL_FACE );
}

void ofApp::DrawScene()
{
    DrawSphereGrid();
 }

void ofApp::DrawSphereGrid()
{
    glCullFace( GL_FRONT );

    int numSpheres = 8;

    float radius = 30.0f;
    float spacing = radius * 2.0f + 15.0f;
    float offset = -numSpheres * spacing * 0.5f;

    for ( int z = 0; z < numSpheres; ++z )
    {
        float zPercent = z / (float)( numSpheres - 1 );

        for ( int x = 0; x < numSpheres; ++x )
        {
            float xPercent = x / (float)( numSpheres - 1 );
            m_material.metallic = std::max( zPercent * zPercent, 0.001f );
            m_material.roughness = std::max( xPercent * xPercent, 0.001f );
            m_material.Bind( m_shader );

            ofPushMatrix();
            ofTranslate( offset + x * spacing, radius * 2.0, offset + z * spacing );
            ofScale( radius );
            m_shader.setUniformMatrix3f( "normalMatrix", lb::GetNormalMatrix() );
            m_sphere.draw();
            ofPopMatrix();
        }
    }

    glCullFace( GL_BACK );
}

//--------------------------------------------------------------
void ofApp::update()
{
//    AnimateLights();

    if ( m_bMouseOverGui ) 
    {
        m_camera.disableMouseInput();
    }
    else 
    {
        m_camera.enableMouseInput();
    }

    m_bMouseOverGui = false;

    if ( ofGetFrameNum() % 2 == 0 )
    {
        m_particleSystem.step( ( 1.0f / 60.0f * 1000.0f ) * 2.0f );
    }

    m_particleSystem.update();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );

    ofClear( ofFloatColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
    ofDisableAlphaBlending();

    m_lightSystem.Bind();
    m_viewUbo.Bind();

    m_skyboxMap.BindTexture( 14 );
    m_irradianceMap.BindTexture( 2 );
    m_radianceMap.BindTexture( 3 );

    switch ( m_appMode )
    {
        case AppMode::NORMAL_VIEW:
        {
            m_camera.begin();
            {
                m_viewUbo.Update( m_camera );
                m_lightSystem.Update( m_camera );

                ofSetColor( 255, 255, 255, 255 );

                m_particleSystem.debugDrawWorldBounds();

                std::vector<lb::PointLight>& pointLights = m_lightSystem.GetPointLights();
                for ( int idx = 0; idx < pointLights.size(); ++idx )
                {
                    lb::PointLight& light = pointLights[ idx ];
                    ofPushMatrix();
                    ofTranslate( light.position.x, light.position.y, light.position.z );
                    ofScale( light.radius * 0.05f );
                    ofSetColor( ofFloatColor( light.color.x, light.color.y, light.color.z ) );
                    m_sphere.draw();
                    ofPopMatrix();
                }
             

                m_particleShader.begin();
                {
                    ofTranslate( 0, 0, 0 );
                    m_particleMaterial.Bind( m_particleShader );
                    m_particleShader.setUniform1f( "uExposure", m_exposure );
                    m_particleShader.setUniform1f( "uGamma", m_gamma );
                    m_particleShader.setUniform1i( "uIrradianceMap", 2 );
                    m_particleShader.setUniform1i( "uRadianceMap", 3 );
                    m_particleShader.setUniformTexture( "uOffsetTex", m_particleSystem.getPositionTexture(), 0 );

                    glEnable( GL_CULL_FACE );
                    m_particleSystem.debugDrawParticles();
                }
                m_particleShader.end();
            }
            m_camera.end();

            imGui();
        }
        break;

        case AppMode::DEBUG_VIEW:
        {
            m_debugCamera.begin();
            {
                m_viewUbo.Update( m_camera );
                m_lightSystem.Update( m_camera );

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
