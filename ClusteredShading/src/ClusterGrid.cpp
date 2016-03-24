#include "ClusterGrid.h"

#include "lb/math/Plane.h"
#include "lb/math/Frustum.h"
#include "lb/math/ProjectionMath.h"
#include "lb/math/SphereScissor.h"
#include "lb/util/RadixSort.h"
#include "lb/gl/GLError.h"

lb::ClusterGrid::ClusterGrid()
{
}

void lb::ClusterGrid::Init( const lb::ProjInfo& _projInfo )
{
    m_projInfo = _projInfo;

    memset( m_culledPointLightIndices, 0, sizeof( m_culledPointLightIndices[0] ) * MAX_POINT_LIGHTS );
    m_numCulledLightIndices = 0;

    memset( m_lightIndices, 0, sizeof( m_lightIndices[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_lightSortKeys, 0, sizeof( m_lightSortKeys[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_tempLightSortKeys, 0, sizeof( m_tempLightSortKeys[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_tempLightIndices, 0, sizeof( m_tempLightIndices[0] ) * MAX_CLUSTERED_LIGHTS );

    memset( m_clusterLightPointerList, 0, sizeof( m_clusterLightPointerList[0] ) * NUM_CLUSTERS );

    CreateLightIndexTextures();
    
    m_planesX = new lb::Plane[ NUM_PLANES_X ];
    m_planesY = new lb::Plane[ NUM_PLANES_Y ];
    m_planesZ = new lb::Plane[ NUM_PLANES_Z ];

    CreatePlanes();
}

lb::ClusterGrid::~ClusterGrid()
{
    delete[] m_planesX;
    delete[] m_planesY;
    delete[] m_planesZ;
}

void lb::ClusterGrid::CreatePlanes()
{
    m_nearPlane = lb::Plane( glm::vec3( 0.0f, 0.0f, -m_projInfo.nearZ ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    m_farPlane  = lb::Plane( glm::vec3( 0.0f, 0.0f, -m_projInfo.farZ ), glm::vec3( 0.0f, 0.0f, 1.0f ) );

    // half height and width in normalized form
    float halfNormHeight = std::tanf( m_projInfo.fov * 0.5f );
    float halfNormWidth = halfNormHeight * m_projInfo.aspectRatio;

    // half height and width at far plane
    float halfFarHeight = halfNormHeight * m_projInfo.farZ;
    float halfFarWidth = halfNormWidth * m_projInfo.farZ;
    float frustumDepth = m_projInfo.farZ - m_projInfo.nearZ;

    // z depth of an individual cluster
    float clusterDepth = frustumDepth / (float)NUM_CLUSTERS_Z;

    float centerZ = -(m_projInfo.farZ - m_projInfo.nearZ) * 0.5f;

    // calculate views space far frustum corner points
    glm::vec3 farTL = glm::vec3( -halfFarWidth,  halfFarHeight, -m_projInfo.farZ );
    glm::vec3 farTR = glm::vec3(  halfFarWidth,  halfFarHeight, -m_projInfo.farZ );
    glm::vec3 farBL = glm::vec3( -halfFarWidth, -halfFarHeight, -m_projInfo.farZ );
    glm::vec3 farBR = glm::vec3(  halfFarWidth, -halfFarHeight, -m_projInfo.farZ );

    // calculate X planes
    glm::vec3 topPoint = farTL;
    glm::vec3 bottomPoint = farBL;

    float stepX = ( halfFarWidth * 2.0f ) / ( NUM_PLANES_X - 1 );

    for ( int x = 0; x < NUM_PLANES_X; ++x )
    {
        // set plane normal and position
        glm::vec3 normal = glm::normalize( glm::cross( topPoint, bottomPoint ) ); 
        m_planesX[ x ]= lb::Plane( normal, 0.0f );

        topPoint.x += stepX;
        bottomPoint.x += stepX;
    }

    // calculate Y planes
    glm::vec3 leftPoint = farBL;
    glm::vec3 rightPoint = farBR;

    float stepY = ( halfFarHeight * 2.0f ) / ( NUM_PLANES_Y - 1 );

    for ( int y = 0; y < NUM_PLANES_Y; ++y )
    {
        // set plane normal and position
        glm::vec3 normal = glm::normalize( glm::cross( leftPoint, rightPoint ) ); 
        m_planesY[ y ]= lb::Plane( normal, 0.0f );

        leftPoint.y += stepY;
        rightPoint.y += stepY;
    }

    // calculate Z planes
    glm::vec3 normal = glm::vec3( 0.0f, 0.0f, -1.0f );
    float distance = -m_projInfo.nearZ;

    for ( int z = 0; z < NUM_PLANES_Z; ++z )
    {
        m_planesZ[ z ] = lb::Plane( normal, distance );
        distance -= clusterDepth;
    }

    // camera frustum planes
    m_frustumPlanes[ 0 ] = m_planesZ[ 0 ];
    m_frustumPlanes[ 1 ] = m_planesZ[ NUM_PLANES_Z - 1 ];

    m_frustumPlanes[ 2 ] = m_planesY[ 0 ];
    m_frustumPlanes[ 3 ] = m_planesY[ NUM_PLANES_Y - 1 ];
    m_frustumPlanes[ 2 ].FlipNormal();

    m_frustumPlanes[ 4 ] = m_planesX[ 0 ];
    m_frustumPlanes[ 4 ].FlipNormal();

    m_frustumPlanes[ 5 ] = m_planesX[ NUM_PLANES_X - 1 ];
}

void lb::ClusterGrid::AddPointLightToCluster( uint16_t light_idx, int x, int y, int z )
{
//    printf( "%u, %u\n", m_numLightIndices, MAX_CLUSTERED_LIGHTS );
    assert( m_numLightIndices < MAX_CLUSTERED_LIGHTS );

    uint16_t clusterId = z * ( NUM_CLUSTERS_X * NUM_CLUSTERS_Y ) + y * NUM_CLUSTERS_X + x;
    assert( clusterId <= CLUSTER_INDEX_MASK );

    m_clusterLightPointerList[ clusterId ].pointLightCount++;

    m_lightIndices[ m_numLightIndices ] = light_idx; // store light index
    m_lightSortKeys[ m_numLightIndices ] = ( clusterId ); // &LIGHT_TYPE_MASK ); // store light key

    ++m_numLightIndices;
}

void lb::ClusterGrid::CreateLightIndexTextures()
{
    // light index TBO
    glGenBuffers( 1, &m_lightIndexTbo );
    glBindBuffer( GL_TEXTURE_BUFFER, m_lightIndexTbo );

    glGenTextures( 1, &m_lightIndexTex );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_BUFFER, m_lightIndexTex );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_R16UI, m_lightIndexTbo );

    glBindBuffer( GL_TEXTURE_BUFFER,  m_lightIndexTbo );
    glBufferData( GL_TEXTURE_BUFFER, MAX_CLUSTERED_LIGHTS * sizeof( m_lightIndices[0] ), NULL, GL_DYNAMIC_DRAW );

    glBindBuffer( GL_TEXTURE_BUFFER, 0 );
    glBindTexture( GL_TEXTURE_BUFFER, 0 );

    // light pointer 3d texture
    // RG32_UI (R = Offset into light index TBO, G = ( PointLightCount << 16 | SpotLightCount )

    lb::CheckGLError();
    glGenTextures( 1, &m_lightPointerTableTex3d );
    glBindTexture(GL_TEXTURE_3D, m_lightPointerTableTex3d );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT );
    lb::CheckGLError();

    glTexImage3D( GL_TEXTURE_3D, 0, GL_RG32UI, NUM_CLUSTERS_X, NUM_CLUSTERS_Y, NUM_CLUSTERS_Z, 0, GL_RG_INTEGER, GL_UNSIGNED_INT, nullptr );
    lb::CheckGLError();
}

void lb::ClusterGrid::CullPointLights( const ofMatrix4x4& _viewMatrix, const std::vector<lb::PointLight>& _pointLights )
{
    // reset clusters
    m_numLightIndices = 0;
    m_numAffectedClusters = 0;

    memset( m_lightIndices, 0, sizeof( m_lightIndices[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_lightSortKeys, 0, sizeof( m_lightSortKeys[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_tempLightSortKeys, 0, sizeof( m_tempLightSortKeys[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_tempLightIndices, 0, sizeof( m_tempLightIndices[0] ) * MAX_CLUSTERED_LIGHTS );
    memset( m_clusterLightPointerList, 0, sizeof( m_clusterLightPointerList[0] ) * NUM_CLUSTERS );

    memset( m_culledPointLightIndices, 0, sizeof( m_culledPointLightIndices[0] ) * MAX_POINT_LIGHTS );
    m_numCulledLightIndices = 0;

    m_numVisibleLights = 0;

    float rcpNearFarLen = 1.0f / ( m_projInfo.farZ - m_projInfo.nearZ );

    float horizFov = lb::GetHorizontalFov( m_projInfo.fov, m_projInfo.aspectRatio );
    float focalLength = lb::GetFocalLength( horizFov );
    float aspectRatio = 1.0f / m_projInfo.aspectRatio; // height / width

    for ( uint16_t light_idx = 0; light_idx < _pointLights.size(); ++light_idx )
    {
        const lb::PointLight& light = _pointLights[ light_idx ];

        ofVec4f posVS = ofVec4f( light.position.x, light.position.y, light.position.z, 1.0f ) * _viewMatrix;
        glm::vec3 lightPosVS = glm::vec3( posVS.x, posVS.y, posVS.z );

        if ( false == lb::SphereInFrustum( m_frustumPlanes, lightPosVS, light.radius ) )
        {
            m_culledPointLightIndices[ m_numCulledLightIndices++ ] = light_idx;
            continue;
        }

        ++m_numVisibleLights;

        // Z bounds of sphere
        float linearMinZ = ( -lightPosVS.z - light.radius - m_projInfo.nearZ ) * rcpNearFarLen;
        float linearMaxZ = ( -lightPosVS.z + light.radius - m_projInfo.nearZ ) * rcpNearFarLen;

        int z0 = std::max( 0, std::min( (int)( linearMinZ * (float)NUM_CLUSTERS_Z ), NUM_CLUSTERS_Z - 1 ) );
        int z1 = std::max( 0, std::min( (int)( linearMaxZ * (float)NUM_CLUSTERS_Z ), NUM_CLUSTERS_Z - 1 ) );

        glm::vec2 rectMin, rectMax;
        lb::SphereProjectionResult result = lb::ProjectSphere( lightPosVS, light.radius, focalLength, aspectRatio, &rectMin, &rectMax );

        // Is sphere behind near plane? If so, let's use a full frustrum box vs reduced sphere since it's difficult to calculate tight culling in this case.
        // Morgan McGuire's method (http://jcgt.org/published/0002/02/05/paper.pdf) does achieve this tight culling on near plane intersection, but at a cost.
        // Perhaps look into it to see if it's worth it? Most likely scene dependent.
        if ( lb::SphereProjectionResult::FULL == result )
        {
            int x0 = 0;
            int x1 = NUM_CLUSTERS_X - 1;

            int y0 = 0;
            int y1 = NUM_CLUSTERS_Y - 1;

            for ( int z = z0; z <= z1; ++z )
            {
                for ( int y = y0; y <= y1; ++y )
                {
                    for ( int x = x0; x <= x1; ++x )
                    {
                        AddPointLightToCluster( light_idx, x, y, z );
                    }
                }
            }

            continue;
        }

        // find minimum and maximum AABB cluster indices from clip coords
        int x0 = std::min( (int)( ( rectMin.x * 0.5f + 0.5f ) * (float)NUM_CLUSTERS_X ), NUM_CLUSTERS_X - 1 );
        int x1 = std::min( (int)( ( rectMax.x * 0.5f + 0.5f ) * (float)NUM_CLUSTERS_X ), NUM_CLUSTERS_X - 1 );

        int y0 = std::min( (int)( ( rectMin.y * 0.5f + 0.5f ) * (float)NUM_CLUSTERS_Y ), NUM_CLUSTERS_Y - 1 );
        int y1 = std::min( (int)( ( rectMax.y * 0.5f + 0.5f ) * (float)NUM_CLUSTERS_Y ), NUM_CLUSTERS_Y - 1 );

        const int center_y = (int)( ( y0 + y1 ) * 0.5f );
        const int center_z = (int)( ( ( linearMaxZ + linearMinZ ) * 0.5f ) * (float)NUM_CLUSTERS_Z );

        for ( int z = z0; z <= z1; ++z )
        {
            glm::vec3 z_light = lightPosVS;
            float z_radius = light.radius;

            if ( z != center_z )
            {
                if ( z < center_z )
                {
                    const lb::Plane& plane = m_planesZ[ z + 1 ];
                    plane.IntersectSphereZ( lightPosVS, light.radius, &z_light, &z_radius );
                }
                else
                {
                    const lb::Plane& plane = m_planesZ[ z ];
                    plane.IntersectSphereZ( lightPosVS, light.radius, &z_light, &z_radius );
                }
            }

            for ( int y = y0; y <= y1; ++y )
            {
                glm::vec3 y_light = z_light;
                float y_radius = z_radius;

                if ( y != center_y )
                {
                    if ( y < center_y )
                    {
                        const lb::Plane& plane = m_planesY[ y + 1 ];
                        if ( false == plane.IntersectSphereY( z_light, z_radius, &y_light, &y_radius ) )
                        {
                            continue;
                        }
                    }
                    else
                    {
                        const lb::Plane& plane = m_planesY[ y ];
                        if ( false == plane.IntersectSphereY( z_light, z_radius, &y_light, &y_radius ) )
                        {
                            continue;
                        }
                    }
                }

                int x = x0;
                do {
                    ++x;
                } while ( x < x1 && -m_planesX[ x ].SignedDistanceXPlane( y_light ) >= y_radius );

                int xs = x1 + 1;
                do {
                     --xs;
                } while ( xs >= x && m_planesX[ xs ].SignedDistanceXPlane( y_light ) >= y_radius );

                for ( --x; x <= xs; x++ )
                {
                    AddPointLightToCluster( light_idx, x, y, z );
                }
            }
        }
    }
}


void lb::ClusterGrid::UpdateLightIndexTextures()
{
    lb::CheckGLError();
    glBindBuffer( GL_TEXTURE_BUFFER, m_lightIndexTbo );
    lb::CheckGLError();

    uint8_t * lightIndexPtr = (uint8_t *)glMapBuffer( GL_TEXTURE_BUFFER, GL_WRITE_ONLY );
    assert( NULL != lightIndexPtr );
    memcpy( lightIndexPtr, m_lightIndices, sizeof( m_lightIndices[ 0 ] ) * m_numLightIndices );

    glUnmapBuffer( GL_TEXTURE_BUFFER );
    glBindBuffer( GL_TEXTURE_BUFFER, 0 );
    lb::CheckGLError();

    glBindTexture( GL_TEXTURE_3D, m_lightPointerTableTex3d );
    glTexSubImage3D( GL_TEXTURE_3D, 0, 0, 0, 0, NUM_CLUSTERS_X, NUM_CLUSTERS_Y, NUM_CLUSTERS_Z, GL_RG_INTEGER, GL_UNSIGNED_INT, (void *)m_clusterLightPointerList );
    lb::CheckGLError();
}

void lb::ClusterGrid::BindLightIndexTextures( GLuint _indexTexUnit, GLuint _pointerTableTexUnit )
{
    glActiveTexture( GL_TEXTURE0 + _indexTexUnit );
    glBindTexture( GL_TEXTURE_BUFFER, m_lightIndexTex ); // per cluster point light index TBO
    lb::CheckGLError();

    glActiveTexture( GL_TEXTURE0 + _pointerTableTexUnit );
    glBindTexture( GL_TEXTURE_3D, m_lightPointerTableTex3d );
    lb::CheckGLError();
}

void lb::ClusterGrid::SortLightIndexList()
{
    // sort the keys and values - this will give us the light indices sorted by cluster type & light type
    lb::RadixSort16<uint16_t>( m_lightSortKeys, m_tempLightSortKeys, m_lightIndices, m_tempLightIndices, m_numLightIndices );

    // update the light index "pointer" list
    uint16_t currKey;
    uint16_t lastKey = 65535; // invalid key value

    for ( int i = 0; i < m_numLightIndices; ++i )
    {
        currKey = m_lightSortKeys[ i ];
        if ( currKey != lastKey )
        {
            m_clusterLightPointerList[ currKey ].indexOffset = i;
            ++m_numAffectedClusters;
        }

        lastKey = currKey;
    }
}

uint32_t lb::ClusterGrid::GetNumPointLights() const
{
    return m_numLightIndices;
}

const uint16_t * lb::ClusterGrid::GetPointLightIndices() const
{
    return m_lightIndices;
}

uint32_t lb::ClusterGrid::GetNumCulledPointLights() const
{
    return m_numCulledLightIndices;
}

const uint16_t * lb::ClusterGrid::GetCulledPointLightIndices() const
{
    return m_culledPointLightIndices;
}

