//
//  NBodySystemOpenCL.hpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-27.
//
//

#pragma once

#include "MSAOpenCL.h"

#include "NBodySystem.h"

namespace entropy
{
    class NBodySystemOpenCL
    : public NBodySystem
    {
    public:
        NBodySystemOpenCL(int numBodies, unsigned int p, unsigned int q);
        virtual ~NBodySystemOpenCL();

        virtual void update(float deltaTime);

        virtual void setSoftening(float softening);
        virtual void setDamping(float damping);

        virtual ofVbo& getVbo();

        virtual float* getArray(ArrayType type);
        virtual void setArray(ArrayType type, const float* data);

        virtual void synchronizeThreads();

    protected: // methods
        NBodySystemOpenCL() {}

        virtual void _initialize(int numBodies);
        virtual void _finalize();

        void _integrateNBodySystem(float deltaTime);

    protected: // data
        msa::OpenCL	_opencl;
        msa::OpenCLProgramPtr _program;
        msa::OpenCLKernelPtr _kernel;
        msa::OpenCLKernelPtr _kernelMT;

        ofVbo _bufferGL[2];
        msa::OpenCLBufferManagedT<float> _bufferCL[2];
        msa::OpenCLBuffer *_vel[2];

        float* _hPos;
        float* _hVel;

        float _softeningSq;
        float _damping;

        unsigned int _currentRead;
        unsigned int _currentWrite;

        unsigned int _p;
        unsigned int _q;
    };
}
