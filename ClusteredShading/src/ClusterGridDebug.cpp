#include "ClusterGridDebug.h"

lb::ClusterGridDebug::ClusterGridDebug()
{
}

lb::ClusterGridDebug::~ClusterGridDebug()
{
}

void lb::ClusterGridDebug::CreateClusterMesh( const lb::ClusterGrid& _clusterGrid, const lb::ProjInfo& _projInfo )
{
    m_numPlanesX = _clusterGrid.GetNumPlanesX();
    m_numPlanesY = _clusterGrid.GetNumPlanesY();
    m_numPlanesZ = _clusterGrid.GetNumPlanesZ();

    m_numClustersX = _clusterGrid.GetNumClustersX();
    m_numClustersY = _clusterGrid.GetNumClustersY();
    m_numClustersZ = _clusterGrid.GetNumClustersZ();
    m_numClusters = m_numClustersX * m_numClustersY * m_numClustersZ;

    // half height and width in normalized form
    float halfNormHeight = std::tanf( _projInfo.fov * 0.5f );
    float halfNormWidth = halfNormHeight * _projInfo.aspectRatio;

    // half height and width at far plane
    float halfFarHeight = halfNormHeight * _projInfo.farZ;
    float halfFarWidth = halfNormWidth * _projInfo.farZ;
    float halfNearHeight = halfNormHeight * _projInfo.nearZ;
    float halfNearWidth = halfNormWidth * _projInfo.nearZ;

    float frustumDepth = _projInfo.farZ - _projInfo.nearZ;

    // calculate views space far frustum corner points
    ofVec3f farTL = ofVec3f( -halfFarWidth,  halfFarHeight, -_projInfo.farZ );
    ofVec3f farTR = ofVec3f(  halfFarWidth,  halfFarHeight, -_projInfo.farZ );
    ofVec3f farBL = ofVec3f( -halfFarWidth, -halfFarHeight, -_projInfo.farZ );
    ofVec3f farBR = ofVec3f(  halfFarWidth, -halfFarHeight, -_projInfo.farZ );
                                                             
    ofVec3f nearTL = ofVec3f( -halfNearWidth,  halfNearHeight, -_projInfo.nearZ );
    ofVec3f nearTR = ofVec3f(  halfNearWidth,  halfNearHeight, -_projInfo.nearZ );
    ofVec3f nearBL = ofVec3f( -halfNearWidth, -halfNearHeight, -_projInfo.nearZ );
    ofVec3f nearBR = ofVec3f(  halfNearWidth, -halfNearHeight, -_projInfo.nearZ );

    // calculate X planes
    float farStepX = ( halfFarWidth * 2.0f ) / ( m_numPlanesX - 1 );
    float farStepY = ( halfFarHeight * 2.0f ) / ( m_numPlanesY - 1 );

    float nearStepX = ( halfNearWidth * 2.0f ) / ( m_numPlanesX - 1 );
    float nearStepY = ( halfNearHeight * 2.0f ) / ( m_numPlanesY - 1 );

    float stepZ = -frustumDepth / ( m_numPlanesZ - 1 );

    glm::vec4 clusterColor( 1.0f, 1.0f, 1.0f, 0.7f );

    std::vector<ofVec3f> vertices;
    std::vector<glm::vec4> colors;

    for ( int z = 0; z < m_numPlanesZ; ++z )
    {
        ofVec3f nearPoint = nearBL;
        ofVec3f farPoint = farBL;

        float zPercent = (float)z / (m_numPlanesZ - 1);

        for ( int y = 0; y < m_numPlanesY; ++y )
        {
            float yPercent = (float)y / m_numPlanesY;

            for ( int x = 0; x < m_numPlanesX; ++x )
            {
                float xPercent = (float)x / m_numPlanesX;

                ofVec3f dir = farPoint - nearPoint;
                float len = dir.length();
                dir.normalize();

                ofVec3f pt = dir * zPercent * len + nearPoint;
                vertices.push_back( pt );

                nearPoint.x += nearStepX;
                farPoint.x += farStepX;
            }

            nearPoint.x = nearBL.x;
            farPoint.x = farBL.x;

            nearPoint.y += nearStepY;
            farPoint.y += farStepY;
        }
    }

    std::vector<ofIndexType> clusterIndices;

    for ( int z = 0; z < m_numClustersZ; ++z )
    {
        for ( int y = 0; y < m_numClustersY; ++y )
        {
            uint32_t z_offset = m_numPlanesX * m_numPlanesY * z;

            for ( int x = 0; x < m_numClustersX; ++x )
            {
                uint32_t y_offset = y * m_numPlanesX;

                uint32_t tl_front = z_offset + y_offset + x;
                uint32_t tr_front = tl_front + 1;
                uint32_t bl_front = z_offset + y_offset + m_numPlanesX + x;
                uint32_t br_front = bl_front + 1;

                uint32_t tl_back = tl_front + m_numPlanesX * m_numPlanesY;
                uint32_t tr_back = tr_front + m_numPlanesX * m_numPlanesY;
                uint32_t bl_back = bl_front + m_numPlanesX * m_numPlanesY;
                uint32_t br_back = br_front + m_numPlanesX * m_numPlanesY;

                // front face
                clusterIndices.push_back( tl_front ); clusterIndices.push_back( tr_front );
                clusterIndices.push_back( tr_front ); clusterIndices.push_back( br_front );
                clusterIndices.push_back( br_front ); clusterIndices.push_back( bl_front );
                clusterIndices.push_back( bl_front ); clusterIndices.push_back( tl_front );

                // back face
                clusterIndices.push_back( tl_back ); clusterIndices.push_back( tr_back );
                clusterIndices.push_back( tr_back ); clusterIndices.push_back( br_back );
                clusterIndices.push_back( br_back ); clusterIndices.push_back( bl_back );
                clusterIndices.push_back( bl_back ); clusterIndices.push_back( tl_back );

                // sides 
                clusterIndices.push_back( tl_front ); clusterIndices.push_back( tl_back );
                clusterIndices.push_back( tr_front ); clusterIndices.push_back( tr_back );
                clusterIndices.push_back( br_front ); clusterIndices.push_back( br_back );
                clusterIndices.push_back( bl_front ); clusterIndices.push_back( bl_back );
            }
        }
    }

    m_clusterVbo.setVertexData( vertices.data(), vertices.size(), GL_STATIC_DRAW );
    m_clusterVbo.setIndexData( clusterIndices.data(), clusterIndices.size(), GL_STATIC_DRAW );
    m_clusterVbo.enableIndices();

/*    gl::VboMesh::Layout clusterLayout = gl::VboMesh::Layout().usage( GL_STATIC_DRAW )
        .attrib( geom::POSITION, 3 );

    std::vector<gl::VboMesh::Layout> clusterLayoutList;
    clusterLayoutList.push_back( clusterLayout );

    m_clusterVboMesh = gl::VboMesh::create( (uint32_t)vertices.size(), GL_LINES, clusterLayoutList, clusterIndices.size(), GL_UNSIGNED_INT );
    m_clusterVboMesh->bufferIndices( clusterIndices.size() * sizeof( uint32_t ), clusterIndices.data() );
    m_clusterVboMesh->bufferAttrib( geom::POSITION, vertices );
    */
    // Frustum lines
    std::vector<ofVec3f> lineVertices;
    std::vector<ofVec3f> lineColors;

    ofVec3f frustumColor( 1.0f, 1.0f, 1.0f );

    // sides
    lineVertices.push_back( nearBL );
    lineVertices.push_back(  farBL );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearBR );
    lineVertices.push_back(  farBR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearTL );
    lineVertices.push_back(  farTL );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearTR );
    lineVertices.push_back(  farTR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    // near plane
    lineVertices.push_back( nearTL );
    lineVertices.push_back( nearTR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearBL );
    lineVertices.push_back( nearBR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearTL );
    lineVertices.push_back( nearBL );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( nearTR );
    lineVertices.push_back( nearBR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    // far plane
    lineVertices.push_back( farTL );
    lineVertices.push_back( farTR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( farBL );
    lineVertices.push_back( farBR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( farTL );
    lineVertices.push_back( farBL );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );

    lineVertices.push_back( farTR );
    lineVertices.push_back( farBR );
    lineColors.push_back( frustumColor );
    lineColors.push_back( frustumColor );


    m_frustumVbo.setVertexData( lineVertices.data(), lineVertices.size(), GL_STATIC_DRAW );

/*    gl::VboMesh::Layout frustumLayout = gl::VboMesh::Layout().usage( GL_STATIC_DRAW )
        .attrib( geom::POSITION, 3 )
        .attrib( geom::COLOR, 3 );

    std::vector<gl::VboMesh::Layout> frustumLayoutList;
    frustumLayoutList.push_back( frustumLayout );

    gl::VboMeshRef lineVboMesh = gl::VboMesh::create( (uint32_t)lineVertices.size(), GL_LINES, frustumLayoutList );
    lineVboMesh->bufferAttrib( geom::POSITION, lineVertices );
    lineVboMesh->bufferAttrib( geom::COLOR, lineColors );

    m_lineBatch = gl::Batch::create( lineVboMesh, gl::getStockShader( gl::ShaderDef().color() ) );
    */
}

void lb::ClusterGridDebug::DrawFrustum( const ofCamera& _camera )
{
    ofSetColor( ofFloatColor( 1.0f, 1.0f, 1.0f, 1.0f ) );

    ofPushMatrix();
        ofMultMatrix( _camera.getModelViewMatrix().getInverse() );
        m_frustumVbo.draw( GL_LINES, 0, 24 );
    ofPopMatrix();
}

void lb::ClusterGridDebug::DrawCluster( const ofCamera& _camera, uint16_t _idx )
{
    if ( _idx > m_numClusters )
    {
        std::cout << "DrawCluster: index is out of range: " << _idx << std::endl;
        return;
    }

    const int numIndicesPerCluster = 24;
    int startOffset = _idx * numIndicesPerCluster;

    ofPushMatrix();
        ofMultMatrix( _camera.getModelViewMatrix().getInverse() );
        m_clusterVbo.drawElements( GL_LINES, numIndicesPerCluster, startOffset );
    ofPopMatrix();

    /*
    gl::pushMatrices();
        mat4 translate = glm::translate( _camera.getEyePoint() );
        mat4 orientation = glm::mat4_cast( _camera.getOrientation() );
        gl::setModelMatrix( translate * orientation );
        gl::draw( m_clusterVboMesh, startOffset, numIndicesPerCluster );
    gl::popMatrices();
    */
}

void lb::ClusterGridDebug::DrawCluster( const ofCamera& _camera, uint16_t _x, uint16_t _y, uint16_t _z )
{
    if ( _x > m_numClustersX || _x < 0 ) std::cout << "DrawCluster: x is out of range: " << _x << std::endl;
    if ( _y > m_numClustersY || _y < 0 ) std::cout << "DrawCluster: y is out of range: " << _y << std::endl;
    if ( _z > m_numClustersZ || _z < 0 ) std::cout << "DrawCluster: z is out of range: " << _z << std::endl;

    uint16_t idx = ( _z * m_numClustersX * m_numClustersY + _y * m_numClustersX + _x );
    DrawCluster( _camera, idx );

    /*
    gl::pushMatrices();
        mat4 translate = glm::translate( _camera.getEyePoint() );
        mat4 orientation = glm::mat4_cast( _camera.getOrientation() );
        gl::setModelMatrix( translate * orientation );
        gl::draw( m_clusterVboMesh, startOffset, numIndicesPerCluster );
    gl::popMatrices();
    */
}

