#pragma once
#include "ClusterGrid.h"

namespace lb {

class ClusterGridDebug
{
public:
    ClusterGridDebug();
    ~ClusterGridDebug();

    void CreateClusterMesh( const lb::ClusterGrid& _clusterGrid, const lb::ProjInfo& _projInfo );
    void DrawFrustum( const ofCamera& _camera );
    void DrawCluster( const ofCamera& _camera, uint16_t _idx );
    void DrawCluster( const ofCamera& _camera, uint16_t _x, uint16_t _y, uint16_t _z );

private:

private:
    uint16_t            m_numPlanesX;
    uint16_t            m_numPlanesY;
    uint16_t            m_numPlanesZ;

    uint16_t            m_numClustersX;
    uint16_t            m_numClustersY;
    uint16_t            m_numClustersZ;
    uint16_t            m_numClusters;

    ofVbo               m_clusterVbo;
    ofVbo               m_frustumVbo;
};
}