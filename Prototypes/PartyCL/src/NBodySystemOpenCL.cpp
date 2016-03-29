//
//  NBodySystemOpenCL.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-27.
//
//

#include "NBodySystemOpenCL.h"

namespace entropy
{
    //--------------------------------------------------------------
    NBodySystemOpenCL::NBodySystemOpenCL(int numBodies, unsigned int p, unsigned int q)
    : NBodySystem(numBodies)
    ,_hPos(0)
    ,_hVel(0)
    ,_currentRead(0)
    ,_currentWrite(1)
    ,_p(p)
    ,_q(q)
    {
        _opencl.setupFromOpenGL();

        _initialize(numBodies);

        string options = "-cl-fast-relaxed-math";
#ifdef TARGET_OSX
        options += " -DMAC";
#endif

        _program = _opencl.loadProgramFromFile("cl/oclNbodyKernel.cl", false, options);
        _kernel = _opencl.loadKernel("integrateBodies_noMT", _program);
        _kernelMT = _opencl.loadKernel("integrateBodies_MT", _program);

        setSoftening(0.00125f);
        setDamping(0.995f);
    }

    //--------------------------------------------------------------
    NBodySystemOpenCL::~NBodySystemOpenCL()
    {
        _finalize();
        _numBodies = 0;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::_initialize(int numBodies)
    {
        if (_bInitialized) return;

        _numBodies = numBodies;

        _hPos = new float[_numBodies*4];
        _hVel = new float[_numBodies*4];

        memset(_hPos, 0, _numBodies*4*sizeof(float));
        memset(_hVel, 0, _numBodies*4*sizeof(float));

        // Create the position buffer objects for rendering.
        // We will compute directly from this memory in OpenCL too.
        for (int i = 0; i < 2; ++i) {
            _bufferGL[i].setVertexData(_hPos, 4, _numBodies, GL_DYNAMIC_DRAW);
            _bufferCL[i].initFromGLObject(_bufferGL[i].getVertId(), 4 * _numBodies, _hPos);

            _vel[i] = _opencl.createBuffer(_numBodies*4*sizeof(float), CL_MEM_READ_WRITE, _hVel);
        }

        _bInitialized = true;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::_finalize()
    {
        if (!_bInitialized) return;

        delete [] _hPos;
        delete [] _hVel;

        _kernel.reset();
        _kernelMT.reset();

        // This probably leaks, but I don't want to mess with the addon right now.
        for (int i = 0; i < 2; ++i) {
//            delete _vel[i];

            _bufferGL[i].clear();
        }

        _bInitialized = false;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::setSoftening(float softening)
    {
        _softeningSq = softening * softening;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::setDamping(float damping)
    {
        _damping = damping;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::update(float deltaTime)
    {
        if (!_bInitialized) return;

        _integrateNBodySystem(deltaTime);
//        _bufferCL[_currentWrite].readFromDevice();
//        _opencl.finish();

        std::swap(_currentRead, _currentWrite);
    }

    //--------------------------------------------------------------
    ofVbo& NBodySystemOpenCL::getVbo()
    {
        return _bufferGL[_currentRead];
    }

    //--------------------------------------------------------------
    float* NBodySystemOpenCL::getArray(ArrayType type)
    {
        if (!_bInitialized) return;

        float *data = 0;
        switch (type)
        {
            default:
            case ARRAY_POSITION:
                _bufferCL[_currentRead].readFromDevice();
                data = &_bufferCL[_currentRead][0];
                break;

            case ARRAY_VELOCITY:
                _vel[_currentRead]->read(data, 0, 4 * sizeof(float) * _numBodies);
                break;
        }

        return data;
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::setArray(ArrayType type, const float* data)
    {
        if (!_bInitialized) return;

        switch (type)
        {
            default:
            case ARRAY_POSITION:
            {
                _bufferGL[_currentRead].setVertexData(data, 4, _numBodies, GL_DYNAMIC_DRAW);
//                _bufferCL[_currentRead].writeToDevice();
//                _opencl.flush();
            }
                break;

            case ARRAY_VELOCITY:
                _vel[_currentRead]->write((float *)data, 0, 4 * sizeof(float) * _numBodies);
                break;
        }       
    }
    
    //--------------------------------------------------------------
    void NBodySystemOpenCL::synchronizeThreads()
    {
        _opencl.finish();
    }

    //--------------------------------------------------------------
    void NBodySystemOpenCL::_integrateNBodySystem(float deltaTime)
    {
        int sharedMemSize = _p * _q * sizeof(cl_float4);  // 4 floats for pos

        // When the numBodies / thread block size is < # multiprocessors
        // (16 on G80), the GPU is underutilized. For example, with 256 threads per
        // block and 1024 bodies, there will only be 4 thread blocks, so the
        // GPU will only be 25% utilized.  To improve this, we use multiple threads
        // per body.  We still can use blocks of 256 threads, but they are arranged
        // in q rows of p threads each.  Each thread processes 1/q of the forces
        // that affect each body, and then 1/q of the threads (those with
        // threadIdx.y==0) add up the partial sums from the other threads for that
        // body.  To enable this, use the "--p=" and "--q=" command line options to
        // this example.  e.g.: "nbody.exe --n=1024 --p=64 --q=4" will use 4
        // threads per body and 256 threads per block. There will be n/p = 16
        // blocks, so a G80 GPU will be 100% utilized.
        msa::OpenCLKernelPtr kernel;
        if (_q == 1) {
            kernel = _kernelMT;
        }
        else {
            kernel = _kernel;
        }

        for (int i = 0; i < 2; ++i) {
            _bufferCL[i].getCLBuffer().lockGLObject();
        }

        kernel->setArg(0, _bufferCL[_currentWrite]);
        kernel->setArg(1, *_vel[_currentWrite]);
        kernel->setArg(2, _bufferCL[_currentRead]);
        kernel->setArg(3, *_vel[_currentRead]);

        kernel->setArg(4, deltaTime);
        kernel->setArg(5, _damping);
        kernel->setArg(6, _softeningSq);

        kernel->setArg(7, _numBodies);
        kernel->setArg(8, NULL, sharedMemSize);

        // Execute the kernel.
        kernel->run2D(_numBodies, _q, _p, _q);

        for (int i = 0; i < 2; ++i) {
            _bufferCL[i].getCLBuffer().unlockGLObject();
        }

//        _opencl.flush();
    }
}
