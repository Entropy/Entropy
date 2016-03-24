#include "LightSystem.h"
#include "glm/glm.hpp"

using namespace glm;

lb::LightSystem::LightSystem()
    : m_pointLightUboBinding( skDefaultPointLightUboBinding )
    , m_lightIndexTexUnit( skDefaultLightIndexTexUnit )
    , m_lightPointerTexUnit( skDefaultLightPointerTexUnit )
{

}

lb::LightSystem::~LightSystem()
{
}

void lb::LightSystem::Init( const ofCamera& _camera )
{
    lb::ProjInfo projInfo;
    projInfo.fov = ofDegToRad( _camera.getFov() );
    projInfo.aspectRatio = _camera.getAspectRatio();
    projInfo.nearZ = _camera.getNearClip();
    projInfo.farZ = _camera.getFarClip();

    m_pointLights.reserve( lb::ClusterGrid::MAX_POINT_LIGHTS );

    m_clusterGrid.Init( projInfo );
    m_clusterGridDebug.CreateClusterMesh( m_clusterGrid, projInfo );

    // Create point light uniform buffer
    m_pointLightUbo.allocate( lb::ClusterGrid::MAX_POINT_LIGHTS * sizeof( lb::PointLight ), nullptr, GL_DYNAMIC_DRAW );
    assert( true == m_pointLightUbo.isAllocated() );

    m_debugSphere = ofSpherePrimitive( 1.0f, 8 );

    m_pointLights.clear();
}

void lb::LightSystem::ConfigureShader( const ofShader& _shader ) const
{
    _shader.begin();
    _shader.bindUniformBlock( m_pointLightUboBinding, "PointLightBlock" );
    _shader.setUniform1i( "uLightPointerTex", m_lightPointerTexUnit );
    _shader.setUniform1i( "uLightIndexTex", m_lightIndexTexUnit );
    _shader.end();
}

void lb::LightSystem::SetPointLightUboBinding( uint8_t _binding ) 
{ 
    m_pointLightUboBinding = _binding; 
}

void lb::LightSystem::SetLightIndexTexUnit( uint8_t _texUnit )
{
    m_lightIndexTexUnit = _texUnit;
}

void lb::LightSystem::SetLightPointerTexUnit( uint8_t _texUnit )
{
    m_lightPointerTexUnit = _texUnit;
}

void lb::LightSystem::Update( const ofCamera& _camera )
{
    // updateData() will use direct state access (DSA) on GL 4.5, faster than map?
    m_pointLightUbo.updateData( m_pointLights.size() * sizeof( lb::PointLight ), m_pointLights.data() );

    m_clusterGrid.CullPointLights( _camera.getModelViewMatrix(), m_pointLights );
    m_clusterGrid.SortLightIndexList();
    m_clusterGrid.UpdateLightIndexTextures();
}

void lb::LightSystem::Bind()
{
    m_clusterGrid.BindLightIndexTextures( m_lightIndexTexUnit, m_lightPointerTexUnit );
    m_pointLightUbo.bindBase( GL_UNIFORM_BUFFER, m_pointLightUboBinding );
}

void lb::LightSystem::DebugDrawClusteredPointLights()
{
    const uint32_t numLightIndices = m_clusterGrid.GetNumPointLightIndices();
    const uint16_t * lightIndices = m_clusterGrid.GetPointLightIndices();

    for ( int idx = 0; idx < numLightIndices; ++idx )
    {
        ofPushMatrix();
        {
            lb::PointLight& light = m_pointLights[ lightIndices[ idx ] ];
            ofSetColor( ofFloatColor( light.color.x, light.color.y, light.color.z, 1.0f ) );

            ofTranslate( light.position.x, light.position.y, light.position.z );
            ofScale( light.radius );
            m_debugSphere.drawWireframe();
        }
        ofPopMatrix();
    }
}

void lb::LightSystem::DebugDrawCulledPointLights()
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
            m_debugSphere.drawWireframe();
        }
        ofPopMatrix();
    }
}

void lb::LightSystem::DebugDrawFrustum( const ofCamera& _camera )
{
    m_clusterGridDebug.DrawFrustum( _camera );
}

void lb::LightSystem::DebugDrawOccupiedClusters( const ofCamera& _camera )
{
    m_clusterGridDebug.DrawOccupiedClusters( _camera, m_clusterGrid );
}


