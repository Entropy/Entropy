#pragma once

#include "glm/glm.hpp"
#include "ofMain.h"

struct Particle
{
    glm::vec3 position;
    float     mass;
    glm::vec3 velocity;
    float     radius;
};

struct ParticleTboData
{
    glm::mat4 transform;
};

struct ParticleBin
{
    uint16_t offset;
    uint16_t particleCount;
};

struct Attractor
{
    glm::vec3 position;
    float     strength;
};

struct Repeller
{
    glm::vec3 position;
    float     strength;
};

class ParticleSystem
{
    static const uint16_t BIN_DIMS_X = 20;
    static const uint16_t BIN_DIMS_Y = 20;
    static const uint16_t BIN_DIMS_Z = 20;
    static const uint16_t NUM_BINS = BIN_DIMS_X * BIN_DIMS_Y * BIN_DIMS_Z;

    static const uint16_t MAX_PARTICLES = 40000;
    static const uint16_t MAX_ATTRACTORS = 10;
    static const uint16_t MAX_REPELLERS = 10;

public:
    ParticleSystem();
    ~ParticleSystem();

    void init( int _width, int _height, int _depth );
    void shutdown();

    void sortParticlesByBin();

    void step( float _dt );
    void update();

    void addParticle( const glm::vec3& _pos, const glm::vec3& _vel, float _mass, float _radius );
    void addAttractor( const glm::vec3& _pos, float _strength );
    void addRepeller( const glm::vec3& _pos, float _strength );

    inline const ofTexture& getPositionTexture() const { return m_positionTboTex; };

    inline uint16_t binIdFromXYZ( uint16_t _x, uint16_t _y, uint16_t _z )
    {
        return _z * BIN_DIMS_X * BIN_DIMS_Y + _y * BIN_DIMS_X + _x;
    }

    void debugDrawWorldBounds();
    void debugDrawParticles();

    float           m_halfWidth;
    float           m_halfHeight;
    float           m_halfDepth;

    Particle *      m_particlePool;
    Attractor       m_attractors[ MAX_ATTRACTORS ];
    Repeller        m_repellers[ MAX_REPELLERS ];

    uint32_t        m_numParticles;
    uint32_t        m_numAttractors;
    uint32_t        m_numRepellers;

    glm::vec3       m_invBoundsScale;
    glm::vec3       m_binScale;

    ofBoxPrimitive  m_debugBoundsBox;
    ofVboMesh       m_sphereMesh;
    ofBoxPrimitive  m_sphere;

    ofTexture       m_positionTboTex;
    ofBufferObject  m_positionTbo;
    ParticleTboData * m_positions;

    ParticleBin                m_particleBins[ NUM_BINS ];
    uint16_t                   m_particleIndices[ MAX_PARTICLES ]; // unsorted list of particle indices (just particle pool index)
    uint16_t                   m_particleSortKeys[ MAX_PARTICLES ]; // list of bin keys (bin ID)

    uint16_t                   m_tempParticleIndices[ MAX_PARTICLES ]; // temp list for radix sort
    uint16_t                   m_tempParticleSortKeys[ MAX_PARTICLES ]; // temp list for radix sort


};
