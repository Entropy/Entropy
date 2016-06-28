#pragma once

#include "ofMain.h"

struct Particle
{
	enum Type
	{
		ELECTRON,
		POSITRON,

		ANTI_UP_QUARK,
		UP_QUARK,

		NUM_TYPES
	};

	static const float MASSES[NUM_TYPES];
	static const float CHARGES[NUM_TYPES];

	Type type;
    ofVec3f position;
    ofVec3f velocity;
	bool alive;
	// save these members rather than using
	// type to eliminate array lookups in
	// step() function
    float radius;
    float mass;
	float charge;
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
    static const uint16_t MAX_PARTICLES = 10000;
    static const uint16_t MAX_ATTRACTORS = 10;
    static const uint16_t MAX_REPELLERS = 10;
	static const float MIN_SPEED_SQUARED;

    ParticleSystem();
    ~ParticleSystem();

    void init( int _width, int _height, int _depth );
    void shutdown();

    void sortParticlesByBin();

    void step( float _dt );
    void update();

    void addParticle(Particle::Type type, const ofVec3f& _pos, const ofVec3f& _vel, float _mass, float _radius, float charge);
    void addAttractor( const ofVec3f& _pos, float _strength );
    void addRepeller( const ofVec3f& _pos, float _strength );
	void removeParticle(unsigned idx);

    //inline const ofTexture& getPositionTexture() const { return m_positionTboTex; };
    inline const ofTexture& getPositionTexture(Particle::Type type) const { return m_positionTboTex[type]; };

    inline uint16_t binIdFromXYZ( uint16_t _x, uint16_t _y, uint16_t _z )
    {
        return _z * BIN_DIMS_X * BIN_DIMS_Y + _y * BIN_DIMS_X + _x;
    }

	uint32_t getNumParticlesTotal() const { return numParticlesTotal; }

    void debugDrawWorldBounds();
    void debugDrawParticles(Particle::Type type);

private:
    float           m_halfWidth;
    float           m_halfHeight;
    float           m_halfDepth;

    Particle *      m_particlePool;
    Attractor       m_attractors[ MAX_ATTRACTORS ];
    Repeller        m_repellers[ MAX_REPELLERS ];

	uint32_t        numParticlesTotal;
	uint32_t        numParticles[Particle::NUM_TYPES];
    uint32_t        m_numAttractors;
    uint32_t        m_numRepellers;

    ofVec3f       m_invBoundsScale;
    ofVec3f       m_binScale;

    ofBoxPrimitive  m_debugBoundsBox;

	ofVboMesh particleMeshes[Particle::NUM_TYPES];

    ofTexture       m_positionTboTex[Particle::NUM_TYPES];
    ofBufferObject  m_positionTbo[Particle::NUM_TYPES];
    ParticleTboData* m_positions[Particle::NUM_TYPES];

	uint32_t deadParticles[MAX_PARTICLES]; // particles to remove
	uint32_t numDeadParticles;

    ParticleBin                m_particleBins[ NUM_BINS ];
    uint16_t                   m_particleIndices[ MAX_PARTICLES ]; // unsorted list of particle indices (just particle pool index)
    uint16_t                   m_particleSortKeys[ MAX_PARTICLES ]; // list of bin keys (bin ID)

    uint16_t                   m_tempParticleIndices[ MAX_PARTICLES ]; // temp list for radix sort
    uint16_t                   m_tempParticleSortKeys[ MAX_PARTICLES ]; // temp list for radix sort
};
