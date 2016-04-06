#pragma once

#include "lb/lighting/PointLight.h"
#include "lb/lighting/ClusterGrid.h"
#include "lb/lighting/ClusterGridDebug.h"

namespace lb
{
class LightSystem
{
private:
    static const uint8_t skDefaultPointLightUboBinding = 5;
    static const uint8_t skDefaultLightIndexTexUnit = 10;
    static const uint8_t skDefaultLightPointerTexUnit = 11;

public:
    LightSystem();
    ~LightSystem();

    void    Init( const ofCamera& _camera );
    void    ConfigureShader( const ofShader& _shader ) const;

    void    Update( const ofCamera& _camera );
    void    SetPointLightUboBinding( uint8_t _binding );
    void    SetLightIndexTexUnit( uint8_t _texUnit );
    void    SetLightPointerTexUnit( uint8_t _texUnit );

    void    Bind();

    void    DebugDrawCulledPointLights();
    void    DebugDrawClusteredPointLights();
    void    DebugDrawFrustum( const ofCamera& _camera );
    void    DebugDrawOccupiedClusters( const ofCamera& _camera );

    inline uint8_t GetPointLightUboBinding() const  { return m_pointLightUboBinding; }
    inline uint8_t GetLightIndexTexUnit() const     { return m_lightIndexTexUnit; }
    inline uint8_t GetLightPointerTexUnit() const   { return m_lightPointerTexUnit; }
    inline uint32_t GetNumVisibleLights() const     { return m_clusterGrid.GetNumVisibleLights(); }

    inline std::vector<lb::PointLight>& GetPointLights() { return m_pointLights; };

private:
    std::vector<lb::PointLight> m_pointLights;
    lb::ClusterGrid             m_clusterGrid;
    lb::ClusterGridDebug        m_clusterGridDebug;

    ofBufferObject              m_pointLightUbo;
    uint8_t                     m_pointLightUboBinding;
    uint8_t                     m_lightIndexTexUnit;
    uint8_t                     m_lightPointerTexUnit;

    ofSpherePrimitive           m_debugSphere;
};
}
