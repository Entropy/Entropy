#pragma once

#include "glm/glm.hpp"
#include "ofMain.h"

namespace lb {

inline void GetNearFarFromProjectionMatrix( const glm::mat4& _matrix, float* _near, float* _far )
{
//   fNear = -mProj.r3.z / mProj.r2.z;
//   fFar = mProj.r2.z / (mProj.r2.z-1) * fNear;

    float m31 = _matrix[ 0 ][ 2 ];
    float m32 = _matrix[ 1 ][ 2 ];
    float m33 = _matrix[ 2 ][ 2 ];
    float m34 = _matrix[ 3 ][ 2 ];

    float m41 = _matrix[ 0 ][ 3 ];
    float m42 = _matrix[ 1 ][ 3 ];
    float m43 = _matrix[ 2 ][ 3 ];
    float m44 = _matrix[ 3 ][ 3 ];

    glm::vec4 nearPlane;
    glm::vec4 farPlane;

    nearPlane.x = m41 + m31;
    nearPlane.y = m42 + m32;
    nearPlane.z = m43 + m33;
    nearPlane.w = m44 + m34;
    nearPlane = glm::normalize( nearPlane );
    
    farPlane.x = m41 - m31;
    farPlane.y = m42 - m32;
    farPlane.z = m43 - m33;
    farPlane.w = m44 - m34;
    farPlane = glm::normalize( farPlane );
    
    *_near = nearPlane.w / nearPlane.z;
    *_far = farPlane.w / farPlane.z;
}

inline float GetFovFromProjMatrix( const glm::mat4& _matrix )
{
    return 2.0f * atanf( 1.0f / _matrix[ 1 ][ 1 ] );
}

inline float GetHorizontalFov( float _vertFovRad, float _aspectRatio )
{
    // convert vertical FOV to clip, correct with aspect ratio and then back to angle
    return 2.0f * atanf( tanf( _vertFovRad * 0.5f ) * _aspectRatio );
}

inline float GetFocalLength( float _fovRad )
{
    return  1.0f / tanf( _fovRad * 0.5f );
}
  
inline ofMatrix3x3 Mat4ToMat3( const ofMatrix4x4& mat4 )
{
    return ofMatrix3x3(
        mat4._mat[ 0 ][ 0 ], mat4._mat[ 0 ][ 1 ], mat4._mat[ 0 ][ 2 ],
        mat4._mat[ 1 ][ 0 ], mat4._mat[ 1 ][ 1 ], mat4._mat[ 1 ][ 2 ],
        mat4._mat[ 2 ][ 0 ], mat4._mat[ 2 ][ 1 ], mat4._mat[ 2 ][ 2 ] );
}

inline ofMatrix3x3 GetNormalMatrix()
{
    return Mat4ToMat3( ofGetCurrentNormalMatrix() );
}

} // lb namespace end
