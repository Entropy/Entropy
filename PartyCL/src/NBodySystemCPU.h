//
//  NBodySystemCPU.hpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#pragma once

#include "NBodySystem.h"

namespace entropy
{
    class NBodySystemCPU
    : public NBodySystem
    {
    public:
        NBodySystemCPU(int numBodies);
        virtual ~NBodySystemCPU();

        virtual void update(float deltaTime);

        virtual void setSoftening(float softening)
        { _softeningSquared = softening * softening; }
        virtual void setDamping(float damping)
        { _damping = damping; }

        virtual ofVbo& getVbo();

        virtual float* getArray(ArrayType type);
        virtual void setArray(ArrayType type, const float *data);

    protected: // methods
        NBodySystemCPU() {} // default constructor

        virtual void _initialize(int numBodies);
        virtual void _finalize();

        void _bodyBodyInteraction(float accel[3], float posMass0[4], float posMass1[4], float softeningSquared);
        void _computeNBodyGravitation();
        void _integrateNBodySystem(float deltaTime);

    protected: // data
        float* _pos[2];
        float* _vel[2];
        float* _force;

        ofVbo _vbo;

        float _softeningSquared;
        float _damping;

        unsigned int _currentRead;
        unsigned int _currentWrite;
    };
}

