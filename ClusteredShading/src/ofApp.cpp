#include "ofApp.h"
#include "lb/gl/GLError.h"
#include "glm/glm.hpp"

using namespace glm;

ofMatrix3x3 mat4ToMat3( const ofMatrix4x4& mat4 )
{
    return ofMatrix3x3(
        mat4._mat[ 0 ][ 0 ], mat4._mat[ 0 ][ 1 ], mat4._mat[ 0 ][ 2 ],
        mat4._mat[ 1 ][ 0 ], mat4._mat[ 1 ][ 1 ], mat4._mat[ 1 ][ 2 ],
        mat4._mat[ 2 ][ 0 ], mat4._mat[ 2 ][ 1 ], mat4._mat[ 2 ][ 2 ] );
}

//--------------------------------------------------------------
void ofApp::setup()
{
    lb::CheckGLError(); // clear GLEW errors
    m_camera.setupPerspective( false, 60.0f, 1.0f, 1000.0f );
    m_debugCamera.setupPerspective( false, 60.0f, 1.0f, 10000.0f );

    m_camera.setAspectRatio( ( ofGetWindowWidth() * 0.5f ) / ofGetWindowHeight() );
    m_debugCamera.setAspectRatio( ( ofGetWindowWidth() * 0.5f ) / ofGetWindowHeight() );
    
    m_camera.setPosition( ofVec3f( 0.0, 0.0, 623.0 ) );
    m_camera.lookAt( ofVec3f( 0.0, 0.0, 0.0 ), ofVec3f( 0.0f, 1.0f, 0.0f ) );

    SetupLights();
}

void ofApp::SetupLights()
{
    lb::ProjInfo projInfo;
    projInfo.fov = ofDegToRad( m_camera.getFov() );
    projInfo.aspectRatio = m_camera.getAspectRatio();
    projInfo.nearZ = m_camera.getNearClip();
    projInfo.farZ = m_camera.getFarClip();

    m_clusterGrid.Init( projInfo );

    float positionDist = 190.0f;
    float radius = 100.0f;

    lb::PointLight l( vec3( 0.0f, 0.0f, positionDist ), vec3( 1.0f, 0.0f, 0.0f ), radius, 500.0f );
    m_pointLights.push_back( l );

    for ( int i = 0; i < 60; ++i )
    {
        glm::vec3 offset = glm::normalize( vec3( ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ), ofRandom( -1.0f, 1.0f ) ) ) * positionDist;
        lb::PointLight l( offset, vec3( 1.0f, 0.0f, 0.0f ), radius, i * 20.0f + 80.0f );
        l.color = vec4( glm::normalize( vec3( ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ), ofRandom( 0.0f, 1.0f ) ) ), 1.0f );
        m_pointLights.push_back( l );
    }

    m_clusterGridDebug.CreateClusterMesh( m_clusterGrid, projInfo );
    m_sphere = ofSpherePrimitive( 1.0f, 8 );

    m_pointLightUbo.allocate( lb::ClusterGrid::MAX_POINT_LIGHTS * sizeof( lb::PointLight ), nullptr, GL_DYNAMIC_DRAW );
    assert( true == m_pointLightUbo.isAllocated() );
    m_pointLightUbo.bind( GL_UNIFORM_BUFFER );

    m_shader.load( "shaders/passthrough.vert", "shaders/clustered_lights.frag" );
    m_shader.bindUniformBlock( 0, "PointLightBlock" );
    m_shader.printActiveUniforms();
    m_shader.printActiveUniformBlocks();
}

void ofApp::UpdatePointLightUBO()
{
    // updateData() will use direct state access (DSA) on GL 4.5, faster than map?
    m_pointLightUbo.updateData( m_pointLights.size() * sizeof( lb::PointLight ), m_pointLights.data() );
}

void ofApp::AnimateLights()
{
    for ( int idx = 0; idx < m_pointLights.size(); ++idx )
    {
        lb::PointLight& light = m_pointLights[ idx ];
        light.intensity = ( sinf( ( ofGetElapsedTimeMillis() + idx * 20 ) / 100.0f ) * 0.5f + 0.5f ) * 1000.0f; // ( idx / (float)m_pointLights.size() ) * 1000.0f;
    }
}

void ofApp::DrawAllPointLights()
{
    for ( int idx = 0; idx < m_pointLights.size(); ++idx )
    {
        ofPushMatrix();
        {
            lb::PointLight& light = m_pointLights[ idx ];
            ofTranslate( light.position.x, light.position.y, light.position.z );
            ofScale( light.radius * 0.1f );

            ofMatrix3x3 normalMatrix = mat4ToMat3( ofGetCurrentMatrix( OF_MATRIX_MODELVIEW ) );
            normalMatrix.invert();
            normalMatrix.transpose();
            m_shader.setUniformMatrix3f( "normalMatrix", normalMatrix );
            m_sphere.draw();
        }
        ofPopMatrix();
    }

    // Draw big test sphere for something to shade
    ofPushMatrix();
        ofScale( 175.0f );
        ofMatrix3x3 normalMatrix = mat4ToMat3( ofGetCurrentMatrix( OF_MATRIX_MODELVIEW ) );
        normalMatrix.invert();
        normalMatrix.transpose();
        m_shader.setUniformMatrix3f( "normalMatrix", normalMatrix );
        m_sphere.draw();
    ofPopMatrix();
 
}

void ofApp::DrawClusteredPointLights()
{
    const uint32_t numLightIndices = m_clusterGrid.GetNumPointLights();
    const uint16_t * lightIndices = m_clusterGrid.GetPointLightIndices();

    for ( int idx = 0; idx < numLightIndices; ++idx )
    {
        ofPushMatrix();
        {
            lb::PointLight& light = m_pointLights[ lightIndices[ idx ] ];
            ofSetColor( ofFloatColor( light.color.x, light.color.y, light.color.z, 1.0f ) );

            ofTranslate( light.position.x, light.position.y, light.position.z );
            ofScale( light.radius );
            m_sphere.drawWireframe();
        }
        ofPopMatrix();
    }
}

void ofApp::DrawCulledPointLights()
{
    const uint32_t numLightIndices = m_clusterGrid.GetNumCulledPointLights();
    const uint16_t * lightIndices = m_clusterGrid.GetCulledPointLightIndices();

    for ( int idx = 0; idx < numLightIndices; ++idx )
    {
        ofPushMatrix();
        {
            lb::PointLight& light = m_pointLights[ lightIndices[ idx ] ];
            ofSetColor( ofFloatColor( 0.2f, 0.2f, 0.2f, 1.0f ) );

            ofTranslate( light.position.x, light.position.y, light.position.z );
            ofScale( light.radius );
            m_sphere.drawWireframe();
        }
        ofPopMatrix();
    }
}

void ofApp::DrawOccupiedClusters()
{
    static uint16_t idx = 0;

    ofSetColor( ofFloatColor( 1.0f, 1.0f, 1.0f, 0.3f ) );
    for ( uint16_t idx = 0; idx < m_clusterGrid.GetNumClusters(); ++idx )
    {
        if ( m_clusterGrid.m_clusterLightPointerList[ idx ].pointLightCount > 0 )
        {
            m_clusterGridDebug.DrawCluster( m_camera, idx );
        }
    }
}

//--------------------------------------------------------------
void ofApp::update()
{
    m_camera.setOrientation( ofVec3f( 0.0f, sinf( ofGetElapsedTimeMillis() / 10000.0f ) * 65.0f, 0.0f ) );

    LARGE_INTEGER frequency; // ticks per second
    LARGE_INTEGER clusterBeginTime, clusterEndTime;
    QueryPerformanceFrequency( &frequency );

    QueryPerformanceCounter( &clusterBeginTime );
    m_clusterGrid.CullPointLights( m_camera.getModelViewMatrix(), m_pointLights );
    QueryPerformanceCounter( &clusterEndTime ); // stop timer

    // compute and print the elapsed time in millisec
    double clusterTime = ( clusterEndTime.QuadPart - clusterBeginTime.QuadPart ) * 1000.0 / frequency.QuadPart;

    LARGE_INTEGER sortBeginTime, sortEndTime;           // ticks
    QueryPerformanceCounter( &sortBeginTime );
    m_clusterGrid.SortLightIndexList();
    QueryPerformanceCounter( &sortEndTime );
    double sortTime = ( sortEndTime.QuadPart - sortBeginTime.QuadPart ) * 1000.0 / frequency.QuadPart;

    m_clusterGrid.UpdateLightIndexTextures();

    AnimateLights();
    UpdatePointLightUBO();

    if ( ofGetFrameNum() % 5 == 0 )
    {
        printf( "Clustering Time: %f ms, Sort Time: %f ms\n", clusterTime, sortTime );
        printf( "    Num Lights in view: %u, Affected Clusters: %u\n\n", m_clusterGrid.m_numVisibleLights, m_clusterGrid.m_numAffectedClusters );
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
   // glEnable( GL_CULL_FACE );
    //glCullFace( GL_BACK );

    m_pointLightUbo.bindBase( GL_UNIFORM_BUFFER, 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_BUFFER, m_clusterGrid.m_lightIndexTex ); // per cluster point light index TBO

    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_3D, m_clusterGrid.m_lightPointerTableTex3d );


    ofClear( ofFloatColor( 0.1f, 0.1f, 0.1f, 1.0f ) );

    ofEnableDepthTest();
    ofViewport( 0.0f, 0.0f, ofGetWindowWidth() * 0.5f, ofGetWindowHeight() );
    m_camera.begin();
    {
        ofSetColor( 255, 255, 255, 255 );
        ofDrawAxis( 100 );

        m_shader.begin();

            m_shader.setUniform1i( "uLightIndexTex", 0 );
            m_shader.setUniform1i( "uLightPointerTex", 1 );
            m_shader.setUniform1f( "uNumClusters", m_clusterGrid.m_numAffectedClusters );
            m_shader.setUniform1f( "uNearClip", m_camera.getNearClip() );
            m_shader.setUniform1f( "uFarClip", m_camera.getFarClip() );

            DrawAllPointLights();

        m_shader.end();
     }
    m_camera.end();


    ofViewport( ofGetWindowWidth() * 0.5, 0.0f, ofGetWindowWidth() * 0.5f, ofGetWindowHeight() );
    m_debugCamera.begin();
    {
        ofSetColor( 255, 255, 255, 255 );
        m_clusterGridDebug.DrawFrustum( m_camera );
        
        DrawCulledPointLights();
        DrawClusteredPointLights();
        DrawOccupiedClusters();
    }
    m_debugCamera.end();


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
