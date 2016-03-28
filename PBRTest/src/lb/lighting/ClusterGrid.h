#pragma once
#include "ofMain.h"
#include "glm/glm.hpp"

#include "lb/lighting/PointLight.h"
#include "lb/math/Plane.h"
#include "lb/ProjInfo.h"

namespace lb {

// R32G32_UINT
struct ClusterLightPointer
{
    uint32_t indexOffset;
    uint32_t pointLightCount;
};

class ClusterGrid
{
public:
    static const int NUM_PLANES_X = 21;
    static const int NUM_PLANES_Y = 12;
    static const int NUM_PLANES_Z = 17;

    static const int NUM_CLUSTERS_X = NUM_PLANES_X - 1;
    static const int NUM_CLUSTERS_Y = NUM_PLANES_Y - 1;
    static const int NUM_CLUSTERS_Z = NUM_PLANES_Z - 1;

    static const uint16_t NUM_CLUSTERS = NUM_CLUSTERS_X * NUM_CLUSTERS_Y * NUM_CLUSTERS_Z;
    static const uint16_t MAX_CLUSTERED_LIGHTS = 0xffff;  // 65535 max clustered lights 

    static const uint16_t CLUSTER_INDEX_MASK     = 0x0fff; // (0111111111111111) 15 bit mask (0-4096 clusters) 
    static const uint16_t LIGHT_TYPE_MASK        = 0xf000; // (1000000000000000) 1 bit mask (point light or spot light) 
    static const uint8_t  POINT_LIGHT_TYPE_VALUE = 0;
    static const uint8_t  SPOT_LIGHT_TYPE_VALUE  = 1;

    static const uint32_t MAX_POINT_LIGHTS = 1024;

public:
    ClusterGrid();
    ~ClusterGrid();
    
    void Init( const lb::ProjInfo& _projInfo );

    void CullPointLights( const ofMatrix4x4& _viewMatrix, const std::vector<lb::PointLight>& _pointLights );

    void SortLightIndexList();

    void UpdateLightIndexTextures();
    void BindLightIndexTextures( GLuint _indexTexUnit, GLuint _pointerTableTexUnit );

    inline int GetNumPlanesX() const { return NUM_PLANES_X; };
    inline int GetNumPlanesY() const { return NUM_PLANES_Y; };
    inline int GetNumPlanesZ() const { return NUM_PLANES_Z; };

    inline int GetNumClustersX() const { return NUM_CLUSTERS_X; };
    inline int GetNumClustersY() const { return NUM_CLUSTERS_Y; };
    inline int GetNumClustersZ() const { return NUM_CLUSTERS_Z; };
    inline int GetNumClusters()  const { return NUM_CLUSTERS; };

    uint16_t            GetNumVisibleLights() const;

    uint32_t            GetNumPointLightIndices() const;
    const uint16_t *    GetPointLightIndices() const;

    uint32_t            GetNumCulledPointLights() const;
    const uint16_t *    GetCulledPointLightIndices() const;

    uint32_t            GetNumAffectedClusters() const;

private:
    void CreatePlanes();
    void CreateLightIndexTextures();

    void AddPointLightToCluster( uint16_t light_idx, int x, int y, int z );

    lb::ProjInfo    m_projInfo;

    lb::Plane * 	m_planesX;
    lb::Plane * 	m_planesY;
    lb::Plane * 	m_planesZ;

    lb::Plane   	m_nearPlane;
    lb::Plane   	m_farPlane;

    lb::Plane       m_frustumPlanes[ 6 ];

public:
    GLuint                     m_lightIndexTbo;
    GLuint                     m_lightIndexTex;
    GLuint                     m_lightPointerTableTex3d;

    uint16_t                   m_lightIndices[ MAX_CLUSTERED_LIGHTS ]; // unsorted list of light indices (just light IDs)
    uint16_t                   m_lightSortKeys[ MAX_CLUSTERED_LIGHTS ]; // list of light keys (cluster ID and light type encoded in 16 bits)

    uint16_t                   m_tempLightIndices[ MAX_CLUSTERED_LIGHTS ]; // temp list for radix sort
    uint16_t                   m_tempLightSortKeys[ MAX_CLUSTERED_LIGHTS ]; // temp list for radix sort

    lb::ClusterLightPointer    m_clusterLightPointerList[ NUM_CLUSTERS ]; // data for 3d texture (cluster's light index offset and light counts)

    uint32_t                   m_numLightIndices;
    uint32_t                   m_numAffectedClusters;

    uint16_t                   m_culledPointLightIndices[ MAX_POINT_LIGHTS ]; // unsorted list of light indices (just light IDs)
    uint32_t                   m_numCulledLightIndices;

    int                        m_numVisibleLights;
};

} // namespace lb
