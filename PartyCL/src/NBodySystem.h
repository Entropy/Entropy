//
//  NBodySystem.h
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#pragma once

#include "ofMain.h"

namespace entropy
{
    enum NBodyConfig
    {
        NBODY_CONFIG_RANDOM,
        NBODY_CONFIG_SHELL,
        NBODY_CONFIG_EXPAND,

        NBODY_NUM_CONFIGS
    };

    // Utility function.
    void randomizeBodies(NBodyConfig config, float* pos, float* vel, float* color, float clusterScale, float velocityScale, int numBodies);

    class NBodySystem
    {
    public:
        enum ArrayType
        {
            ARRAY_POSITION,
            ARRAY_VELOCITY,
        };

        NBodySystem(int numBodies)
        : _numBodies(numBodies)
        , _bInitialized(false)
        {}

        virtual ~NBodySystem()
        {}

        virtual void update(float dt) = 0;

        virtual void setSoftening(float softening) = 0;
        virtual void setDamping(float damping) = 0;

        virtual ofVbo& getVbo() = 0;

        virtual float* getArray(ArrayType type) = 0;
        virtual void setArray(ArrayType type, const float* data) = 0;

        virtual int getNumBodies() const
        { return _numBodies; }

        virtual void synchronizeThreads()
        {};

    protected: // methods
        NBodySystem();

        virtual void _initialize(int numBodies) = 0;
        virtual void _finalize() = 0;

    protected: // data
        int _numBodies;
        bool _bInitialized;
    };
}
