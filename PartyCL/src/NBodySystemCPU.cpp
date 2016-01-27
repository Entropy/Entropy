//
//  NBodySystemCPU.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-26.
//
//

#include "NBodySystemCPU.h"

namespace entropy
{
    //--------------------------------------------------------------
    NBodySystemCPU::NBodySystemCPU(int numBodies)
    : NBodySystem(numBodies),
    _force(0),
    _softeningSquared(.00125f),
    _damping(0.995f),
    _currentRead(0),
    _currentWrite(1)
    {
        for (int i = 0; i < 2; ++i) {
            _pos[i] = nullptr;
            _vel[i] = nullptr;
        }

        _initialize(numBodies);
    }

    //--------------------------------------------------------------
    NBodySystemCPU::~NBodySystemCPU()
    {
        _finalize();
        _numBodies = 0;
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::_initialize(int numBodies)
    {
        if (_bInitialized) return;

        _numBodies = numBodies;

        for (int i = 0; i < 2; ++i) {
            _pos[i] = new float[_numBodies*4];
            _vel[i] = new float[_numBodies*4];

            memset(_pos[i], 0, _numBodies*4*sizeof(float));
            memset(_vel[i], 0, _numBodies*4*sizeof(float));
        }

        _force  = new float[_numBodies*4];
        memset(_force, 0, _numBodies*4*sizeof(float));

        _vbo.setVertexData(_pos[_currentWrite], 4, _numBodies, GL_DYNAMIC_DRAW);

        _bInitialized = true;
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::_finalize()
    {
        if (!_bInitialized) return;

        for (int i = 0; i < 2; ++i) {
            delete [] _pos[i];
            delete [] _vel[i];

        }

        delete [] _force;

        _vbo.clear();

        _bInitialized = false;
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::update(float deltaTime)
    {
        if (!_bInitialized) return;

        _integrateNBodySystem(deltaTime);

        // Upload data to VBO.
        _vbo.setVertexData(_pos[_currentWrite], 4, _numBodies, GL_DYNAMIC_DRAW);

        std::swap(_currentRead, _currentWrite);
    }

    //--------------------------------------------------------------
    ofVbo& NBodySystemCPU::getVbo()
    {
        return _vbo;
    }

    //--------------------------------------------------------------
    float* NBodySystemCPU::getArray(ArrayType type)
    {
        if (!_bInitialized) return;

        float* data = 0;
        switch (type)
        {
            default:
            case ARRAY_POSITION:
                data = _pos[_currentRead];
                break;

            case ARRAY_VELOCITY:
                data = _vel[_currentRead];
                break;
        }

        return data;
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::setArray(ArrayType type, const float *data)
    {
        if (!_bInitialized) return;

        float* target = 0;
        switch (type)
        {
            default:
            case ARRAY_POSITION:
                target = _pos[_currentRead];
                break;

            case ARRAY_VELOCITY:
                target = _vel[_currentRead];
                break;
        }

        memcpy(target, data, _numBodies*4*sizeof(float));
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::_bodyBodyInteraction(float accel[3], float posMass0[4], float posMass1[4], float softeningSquared)
    {
        float r[3];

        // r_01  [3 FLOPS]
        r[0] = posMass0[0] - posMass1[0];
        r[1] = posMass0[1] - posMass1[1];
        r[2] = posMass0[2] - posMass1[2];

        // d^2 + e^2 [6 FLOPS]
        float distSqr = r[0] * r[0] + r[1] * r[1] + r[2] * r[2];
        distSqr += softeningSquared;

        // invDistCube =1/distSqr^(3/2)  [4 FLOPS (2 mul, 1 sqrt, 1 inv)]
        float invDist = 1.0f / sqrtf(distSqr);
        float invDistCube =  invDist * invDist * invDist;

        // s = m_j * invDistCube [1 FLOP]
        float s = posMass1[3] * invDistCube;

        // (m_1 * r_01) / (d^2 + e^2)^(3/2)  [6 FLOPS]
        accel[0] += r[0] * s;
        accel[1] += r[1] * s;
        accel[2] += r[2] * s;
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::_computeNBodyGravitation()
    {
        for (int i = 0; i < _numBodies; ++i) {
            _force[i*4] = _force[i*4+1] = _force[i*4+2] = 0;

            for (int j = 0; j < _numBodies; ++j) {
                float acc[3] = {0, 0, 0};
                _bodyBodyInteraction(acc, &_pos[_currentRead][j*4], &_pos[_currentRead][i*4], _softeningSquared);

                for (int k = 0; k < 3; ++k) {
                    _force[i*4+k] += acc[k];
                }
            }
        }
    }

    //--------------------------------------------------------------
    void NBodySystemCPU::_integrateNBodySystem(float deltaTime)
    {
        _computeNBodyGravitation();

        for (int i = 0; i < _numBodies; ++i) {
            int index = 4*i;
            float pos[3], vel[3], force[3];
            pos[0] = _pos[_currentRead][index+0];
            pos[1] = _pos[_currentRead][index+1];
            pos[2] = _pos[_currentRead][index+2];
            float mass = _pos[_currentRead][index+3];

            vel[0] = _vel[_currentRead][index+0];
            vel[1] = _vel[_currentRead][index+1];
            vel[2] = _vel[_currentRead][index+2];
            float invMass = _vel[_currentRead][index+3];

            force[0] = _force[index+0];
            force[1] = _force[index+1];
            force[2] = _force[index+2];

            // acceleration = force / mass;
            // new velocity = old velocity + acceleration * deltaTime
            vel[0] += (force[0] * invMass) * deltaTime;
            vel[1] += (force[1] * invMass) * deltaTime;
            vel[2] += (force[2] * invMass) * deltaTime;

            vel[0] *= _damping;
            vel[1] *= _damping;
            vel[2] *= _damping;

            // new position = old position + velocity * deltaTime
            pos[0] += vel[0] * deltaTime;
            pos[1] += vel[1] * deltaTime;
            pos[2] += vel[2] * deltaTime;

            _pos[_currentWrite][index+0] = pos[0];
            _pos[_currentWrite][index+1] = pos[1];
            _pos[_currentWrite][index+2] = pos[2];
            _pos[_currentWrite][index+3] = mass;

            _vel[_currentWrite][index+0] = vel[0];
            _vel[_currentWrite][index+1] = vel[1];
            _vel[_currentWrite][index+2] = vel[2];
            _vel[_currentWrite][index+3] = invMass;
        }
    }

    // Utility function
    //--------------------------------------------------------------
    void randomizeBodies(NBodyConfig config, float* pos, float* vel, float* color, float clusterScale, float velocityScale, int numBodies)
    {
        switch(config)
        {
            default:
            case NBODY_CONFIG_RANDOM:
            {
                float scale = clusterScale * MAX(1.0f, numBodies / (1024.f));
                float vscale = velocityScale * scale;

                int p = 0, v = 0;
                int i = 0;
                while (i < numBodies) {
                    ofVec3f point;
                    //const int scale = 16;
                    point.x = rand() / (float) RAND_MAX * 2 - 1;
                    point.y = rand() / (float) RAND_MAX * 2 - 1;
                    point.z = rand() / (float) RAND_MAX * 2 - 1;
                    float lenSqr = point.dot(point);
                    if (lenSqr > 1)
                        continue;

                    ofVec3f velocity;
                    velocity.x = rand() / (float) RAND_MAX * 2 - 1;
                    velocity.y = rand() / (float) RAND_MAX * 2 - 1;
                    velocity.z = rand() / (float) RAND_MAX * 2 - 1;
                    lenSqr = velocity.dot(velocity);
                    if (lenSqr > 1)
                        continue;

                    pos[p++] = point.x * scale; // pos.x
                    pos[p++] = point.y * scale; // pos.y
                    pos[p++] = point.z * scale; // pos.z
                    pos[p++] = 1.0f; // mass

                    vel[v++] = velocity.x * vscale; // pos.x
                    vel[v++] = velocity.y * vscale; // pos.x
                    vel[v++] = velocity.z * vscale; // pos.x
                    vel[v++] = 1.0f; // inverse mass

                    i++;
                }
            }
                break;

            case NBODY_CONFIG_SHELL:
            {
                float scale = clusterScale;
                float vscale = scale * velocityScale;
                float inner = 2.5f * scale;
                float outer = 4.0f * scale;

                int p = 0, v=0;
                int i = 0;
                while (i < numBodies) {
                    float x, y, z;
                    x = rand() / (float) RAND_MAX * 2 - 1;
                    y = rand() / (float) RAND_MAX * 2 - 1;
                    z = rand() / (float) RAND_MAX * 2 - 1;

                    ofVec3f point = {x, y, z};
                    float len = point.length();
                    point.normalize();
                    if (len > 1)
                        continue;

                    pos[p++] =  point.x * (inner + (outer - inner) * rand() / (float) RAND_MAX);
                    pos[p++] =  point.y * (inner + (outer - inner) * rand() / (float) RAND_MAX);
                    pos[p++] =  point.z * (inner + (outer - inner) * rand() / (float) RAND_MAX);
                    pos[p++] = 1.0f;

                    x = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
                    y = 0.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
                    z = 1.0f; // * (rand() / (float) RAND_MAX * 2 - 1);
                    ofVec3f axis = {x, y, z};
                    axis.normalize();

                    if (1 - point.dot(axis) < 1e-6)
                    {
                        axis.x = point.y;
                        axis.y = point.x;
                        axis.normalize();
                    }
                    //if (point.y < 0) axis = scalevec(axis, -1);

                    ofVec3f vv = {pos[4*i], pos[4*i+1], pos[4*i+2]};
                    vv = vv.cross(axis);
                    vel[v++] = vv.x * vscale;
                    vel[v++] = vv.y * vscale;
                    vel[v++] = vv.z * vscale;
                    vel[v++] = 1.0f;
                    
                    i++;
                }
            }
                break;

            case NBODY_CONFIG_EXPAND:
            {
                float scale = clusterScale * MAX(1.0f, numBodies / (1024.f));
                float vscale = scale * velocityScale;
                
                int p = 0, v = 0;
                int i = 0;
                while (i < numBodies) {
                    ofVec3f point;
                    
                    point.x = rand() / (float) RAND_MAX * 2 - 1;
                    point.y = rand() / (float) RAND_MAX * 2 - 1;
                    point.z = rand() / (float) RAND_MAX * 2 - 1;
                    
                    float lenSqr = point.dot(point);
                    if (lenSqr > 1)
                        continue;
                    
                    pos[p++] = point.x * scale; // pos.x
                    pos[p++] = point.y * scale; // pos.y
                    pos[p++] = point.z * scale; // pos.z
                    pos[p++] = 1.0f; // mass
                    vel[v++] = point.x * vscale; // pos.x
                    vel[v++] = point.y * vscale; // pos.x
                    vel[v++] = point.z * vscale; // pos.x
                    vel[v++] = 1.0f; // inverse mass
                    
                    i++;
                }
            }
                break;
        }
        
        if (color) {
            int v = 0;
            for(int i=0; i < numBodies; i++) {
                //const int scale = 16;
                color[v++] = rand() / (float) RAND_MAX;
                color[v++] = rand() / (float) RAND_MAX;
                color[v++] = rand() / (float) RAND_MAX;
                color[v++] = 1.0f;
            }
        }
    }
}