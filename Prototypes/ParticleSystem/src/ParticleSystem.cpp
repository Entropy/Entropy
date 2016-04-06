#include "ParticleSystem.h"
#include "lb/util/RadixSort.h"

#include "glm/gtx/transform.hpp"

ParticleSystem::ParticleSystem()
    : m_particlePool( nullptr )
    , m_numParticles( 0 )
{}

ParticleSystem::~ParticleSystem()
{}

void ParticleSystem::init( int _width, int _height, int _depth )
{
    m_halfWidth = _width / 2.0f;
    m_halfHeight = _height / 2.0f;
    m_halfDepth = _depth / 2.0f;

    m_particlePool = new Particle[ MAX_PARTICLES ];
    m_positions = new ParticleTboData[ MAX_PARTICLES ]();
    
    memset( m_particlePool, 0, sizeof( m_particlePool[0] ) * MAX_PARTICLES );
    memset( m_positions, 0, sizeof( m_positions[0] ) * MAX_PARTICLES );
    memset( m_particleBins, 0, sizeof( m_particleBins[0] ) * MAX_PARTICLES );

    glm::vec3 minBounds( -m_halfWidth, -m_halfHeight, -m_halfDepth );
    glm::vec3 maxBounds( m_halfWidth, m_halfHeight, m_halfDepth );
    glm::vec3 size = maxBounds - minBounds;

    m_invBoundsScale = 1.0f / ( maxBounds - minBounds );
    m_binScale = glm::vec3( (float)BIN_DIMS_X, (float)BIN_DIMS_Y, (float)BIN_DIMS_Z ) * m_invBoundsScale;

    m_debugBoundsBox.disableNormals();
    m_debugBoundsBox.disableTextures();
    m_debugBoundsBox.setResolution( 1 );
    m_debugBoundsBox.setWidth( size.x );
    m_debugBoundsBox.setHeight( size.y );
    m_debugBoundsBox.setDepth( size.z ); 

    m_sphere = ofBoxPrimitive( 1.0f, 1.0f, 1.0f );
    m_sphereMesh = m_sphere.getMesh();
    m_sphereMesh.setUsage( GL_STATIC_DRAW );

    m_positionTbo.allocate();
    m_positionTbo.bind( GL_TEXTURE_BUFFER );
    m_positionTbo.unbind( GL_TEXTURE_BUFFER );

    m_positionTbo.setData( sizeof( m_positions[ 0 ] ) * MAX_PARTICLES, m_positions, GL_DYNAMIC_DRAW );
    m_positionTboTex.allocateAsBufferTexture( m_positionTbo, GL_RGBA32F );
}

void ParticleSystem::shutdown()
{
    delete[] m_particlePool;
    delete[] m_positions;
}

void ParticleSystem::addParticle( const glm::vec3& _pos, const glm::vec3& _vel, float _mass, float _radius )
{
    Particle& p = m_particlePool[ m_numParticles ];
    p.position = _pos;
    p.velocity = _vel;
    p.mass = _mass;
    p.radius = _radius;

    ++m_numParticles;
}

void ParticleSystem::addAttractor( const glm::vec3& _pos, float _strength )
{
    Attractor& att = m_attractors[ m_numAttractors ];
    att.position = _pos;
    att.strength = _strength;
    ++m_numAttractors;
}

void ParticleSystem::addRepeller( const glm::vec3& _pos, float _strength )
{
    Repeller& rep = m_repellers[ m_numRepellers ];
    rep.position = _pos;
    rep.strength = _strength;
    ++m_numRepellers;
}

void ParticleSystem::sortParticlesByBin()
{
    // sort the keys and values - this will give us the particle indices sorted by bin id 
    lb::RadixSort16<uint16_t>( m_particleSortKeys, m_tempParticleSortKeys, m_particleIndices, m_tempParticleIndices, m_numParticles );
}

void ParticleSystem::update()
{
    memset( m_particleIndices, 0, sizeof( m_particleIndices[ 0 ] ) * MAX_PARTICLES );
    memset( m_particleSortKeys, 0, sizeof( m_particleSortKeys[ 0 ] ) * MAX_PARTICLES );
    memset( m_tempParticleSortKeys, 0, sizeof( m_tempParticleSortKeys[ 0 ] ) * MAX_PARTICLES );
    memset( m_tempParticleIndices, 0, sizeof( m_tempParticleIndices[ 0 ] ) * MAX_PARTICLES );
    memset( m_particleBins, 0, sizeof( m_particleBins[ 0 ] ) * MAX_PARTICLES );

#pragma omp parallel for
    for ( int idx = 0; idx < m_numParticles; ++idx )
    {
        Particle& p = m_particlePool[ idx ];
        p.position += p.velocity;
        p.velocity *= 0.9f;

        // check walls
        if ( p.position.x > m_halfWidth )
        {
            //p.position.x = -m_halfWidth + 10.0f;
            p.position.x = m_halfWidth;
        }
        else if ( p.position.x < -m_halfWidth )
        {
            //p.position.x = m_halfWidth - 10.0f;
            p.position.x = -m_halfWidth;
        }

        if ( p.position.y > m_halfHeight)
        {
            //p.position.y = -m_halfHeight + 10.0f;
            p.position.y = m_halfHeight;
        }
        else if ( p.position.y < -m_halfHeight )
        {
//            p.position.y = m_halfHeight - 10.0f;
            p.position.y = -m_halfHeight;
        }

        if ( p.position.z > m_halfDepth )
        {
            //p.position.z = -m_halfDepth + 10.0f;
            p.position.z = m_halfDepth;
        }
        else if ( p.position.z < -m_halfDepth )
        {
            //p.position.z = m_halfDepth - 10.0f;
            p.position.z = -m_halfDepth;
        }

        const uint16_t px = (uint16_t)floorf( ( p.position.x + m_halfWidth ) * m_binScale.x );
        const uint16_t py = (uint16_t)floorf( ( p.position.y + m_halfHeight ) * m_binScale.y );
        const uint16_t pz = (uint16_t)floorf( ( p.position.z + m_halfDepth ) * m_binScale.z );
        const uint16_t binId = binIdFromXYZ( px, py, pz );

        m_particleIndices[ idx ] = idx;
        m_particleSortKeys[ idx ] = binId;
        ++m_particleBins[ binId ].particleCount;

        //     ofLogNotice() << "bin " << (uint16_t)binId << " ... " << px << ", " << py << ", " << pz << endl;

        ParticleTboData& data = m_positions[ idx ];
        data.transform = glm::translate( p.position )
            * glm::scale( glm::vec3( p.radius, p.radius, p.radius ) );
       //     * glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), p.velocity, glm::vec3( 0.0f, 1.0f, 0.0f ) );
    }

    m_positionTbo.updateData( 0, sizeof( m_positions[ 0 ] ) * m_numParticles, m_positions );
}

void ParticleSystem::step( float _dt )
{
    for ( uint32_t idx = 0; idx < m_numParticles; ++idx )
    {
        Particle& p = m_particlePool[ idx ];

        glm::vec3 acc( 0.0f, 0.0f, 0.0f );
        for ( int attIdx = 0; attIdx < m_numAttractors; ++attIdx )
        {
            const Attractor& att = m_attractors[ attIdx ];
            const float eps = 1e-9f; // minimum dist for nbody interaction

            glm::vec3 delta = att.position - p.position;
            float distSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + eps;
            float invDist = 1.0f / sqrtf( distSqr );
            float invDist3 = invDist * invDist * invDist;
            float f = att.strength * invDist3;

            acc += f * delta;
        }

        for ( int repIdx = 0; repIdx < m_numRepellers; ++repIdx )
        {
            const Repeller& rep = m_repellers[ repIdx ];
            const float eps = 1e-9f; // minimum dist for nbody interaction

            glm::vec3 delta = p.position - rep.position;
            float distSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + eps;
            float invDist = 1.0f / sqrtf( distSqr );
            float invDist3 = invDist * invDist * invDist;
            float f = rep.strength * invDist3;

            acc += f * delta;
        }

        p.velocity += acc / p.mass;
    }

    sortParticlesByBin();

    uint16_t offset = 0;
    for ( uint16_t binIdx = 0; binIdx < NUM_BINS; ++binIdx )
    {
        ParticleBin& bin = m_particleBins[ binIdx ];
        if ( bin.particleCount > 0 )
        {
            bin.offset = offset;
            offset += bin.particleCount;
        }
    }

    // neighbor +/- {range} bins
    const int range = 2;

    // loop through all bins
    for ( int binZ = 0; binZ < BIN_DIMS_Z; ++binZ )
    {
        const uint16_t z0 = ( binZ - range ) < 0 ? 0 : binZ - range;
        const uint16_t z1 = ( binZ + range ) > ( BIN_DIMS_Z - 1 ) ? BIN_DIMS_Z - 1 : binZ + range;

        for ( int binY = 0; binY < BIN_DIMS_Y; ++binY )
        {
            const uint16_t y0 = ( binY - range ) < 0 ? 0 : binY - range;
            const uint16_t y1 = ( binY + range ) > ( BIN_DIMS_Y - 1 ) ? BIN_DIMS_Y - 1 : binY + range;

            for ( int binX = 0; binX < BIN_DIMS_X; ++binX )
            {
                // current bin
                const uint16_t binIdx = binIdFromXYZ( binX, binY, binZ );

                const uint16_t x0 = ( binX - range ) < 0 ? 0 : binX - range;
                const uint16_t x1 = ( binX + range ) > ( BIN_DIMS_X - 1 ) ? BIN_DIMS_X - 1 : binX + range;
                //   ofLogNotice() << "bin " << binX << ", " << binY << ", " << binZ << " min: " << x0 << ", " << y0 << ", " << z0 << " .. max: " << x1 << ", " << y1 << ", " << z1;

                const ParticleBin& bin = m_particleBins[ binIdx ];

                // loop through all particles in bin
#pragma omp parallel for
                for ( int idx = 0; idx < bin.particleCount; ++idx )
                {
                    uint16_t particleIdx = m_particleIndices[ bin.offset + idx ];
                    Particle& p = m_particlePool[ particleIdx ];

                    glm::vec3 acc( 0.0f, 0.0f, 0.0f );

                    // loop through neighbor bins
                    for ( int nz = z0; nz < z1; ++nz )
                        for ( int ny = y0; ny < y1; ++ny )
                            for ( int nx = x0; nx < x1; ++nx )
                            {
                                const ParticleBin& nbin = m_particleBins[ binIdFromXYZ( nx, ny, nz ) ];
                                for ( uint16_t neighborIdx = 0; neighborIdx < nbin.particleCount; ++neighborIdx )
                                {
                                    // n-body euler integration
                                    particleIdx = m_particleIndices[ nbin.offset + neighborIdx ];
                                    Particle& neighborP = m_particlePool[ particleIdx ];

                                    const float eps = 0.1f; // minimum dist for nbody interaction

                                    glm::vec3 delta = neighborP.position - p.position;
                                    float distSqr = delta.x*delta.x + delta.y*delta.y + delta.z*delta.z + eps;
                                    float invDist = 1.0f / sqrtf( distSqr );
                                    float invDist3 = invDist * invDist * invDist;
                                    float f = neighborP.mass * invDist3;

                                    acc += f * delta;
                                }
                            }

                    p.velocity += _dt * acc / p.mass;
                }
            }
        }
    }
}

void ParticleSystem::debugDrawWorldBounds()
{
    ofDisableDepthTest();
    glDisable( GL_CULL_FACE );
    m_debugBoundsBox.drawWireframe();
    ofEnableDepthTest();
}

void ParticleSystem::debugDrawParticles()
{
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    m_sphereMesh.drawInstanced( OF_MESH_FILL, m_numParticles );
}
