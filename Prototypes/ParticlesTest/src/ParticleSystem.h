#pragma once

#include "ofMain.h"

struct Particle
{
    ofVec3f position;
    float     mass;
    ofVec3f velocity;
    float     radius;
};

struct ParticleTboData
{
    ofMatrix4x4 transform;
};

struct ParticleBin
{
    uint16_t offset;
    uint16_t particleCount;
};

struct Attractor
{
    ofVec3f position;
    float     strength;
};

struct Repeller
{
    ofVec3f position;
    float     strength;
};

class ParticleSystem
{
public:
    static const uint16_t BIN_DIMS_X = 20;
    static const uint16_t BIN_DIMS_Y = 20;
    static const uint16_t BIN_DIMS_Z = 20;
    static const uint16_t NUM_BINS = BIN_DIMS_X * BIN_DIMS_Y * BIN_DIMS_Z;
    static const uint16_t MAX_PARTICLES = 2000;
    static const uint16_t MAX_ATTRACTORS = 10;
    static const uint16_t MAX_REPELLERS = 10;

    ParticleSystem();
    ~ParticleSystem();

    void init( int _width, int _height, int _depth );
    void shutdown();

    void sortParticlesByBin();

    void step( float _dt );
    void update();

    void addParticle( const ofVec3f& _pos, const ofVec3f& _vel, float _mass, float _radius );
    void addAttractor( const ofVec3f& _pos, float _strength );
    void addRepeller( const ofVec3f& _pos, float _strength );

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

    ofVec3f       m_invBoundsScale;
    ofVec3f       m_binScale;

    ofBoxPrimitive  m_debugBoundsBox;
    //ofVboMesh       m_sphereMesh;
	ofVboMesh cubeFilletMesh;
   //ofBoxPrimitive  m_sphere;

    ofTexture       m_positionTboTex;
    ofBufferObject  m_positionTbo;
    ParticleTboData* m_positions;

    ParticleBin                m_particleBins[ NUM_BINS ];
    uint16_t                   m_particleIndices[ MAX_PARTICLES ]; // unsorted list of particle indices (just particle pool index)
    uint16_t                   m_particleSortKeys[ MAX_PARTICLES ]; // list of bin keys (bin ID)

    uint16_t                   m_tempParticleIndices[ MAX_PARTICLES ]; // temp list for radix sort
    uint16_t                   m_tempParticleSortKeys[ MAX_PARTICLES ]; // temp list for radix sort


};
