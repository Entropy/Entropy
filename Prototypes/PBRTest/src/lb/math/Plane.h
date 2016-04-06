#pragma once

#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include <math.h>

namespace lb
{
struct Plane
{
    Plane() {};

    Plane( const glm::vec3& _normalizedNormal, float _distance )
        : a( _normalizedNormal.x )
        , b( _normalizedNormal.y )
        , c( _normalizedNormal.z )
        , d( _distance )
    {};

    Plane( const glm::vec3& _normalizedNormal, const glm::vec3& _point )
        : a( _normalizedNormal.x )
        , b( _normalizedNormal.y )
        , c( _normalizedNormal.z )
    {
        d = -glm::dot( _normalizedNormal, _point );
    }

    Plane( const glm::vec3& _point, const glm::vec3& _v1, const glm::vec3& _v2 )
    {
        Plane( glm::cross( _v1, _v2 ), _point );
    }

    Plane( const Plane& _plane )
        : a( _plane.a )
        , b( _plane.b )
        , c( _plane.c )
        , d( _plane.d )
    {}

    Plane& operator=( const Plane& _plane )
    {
        a = _plane.a;
        b = _plane.b;
        c = _plane.c;
        d = _plane.d;

        return *this;
    }

    glm::vec3 Normal() const
    {
        return glm::vec3( a, b, c );
    }

    void Normalize()
    {
        float invLen = 1.0f / sqrtf( a*a + b*b + c*c );
        a *= invLen;
        b *= invLen;
        c *= invLen;
        d *= invLen;
    }
    
    float SignedDistance( const glm::vec3& _point ) const
    {
        return a * _point.x + b * _point.y + c * _point.z + d;
    }

    float SignedDistanceXPlane( const glm::vec3& _point ) const
    {
        return a * _point.x + c * _point.z + d;
    }

    float UnsignedDistance( const glm::vec3& _point ) const
    {
        return fabsf( a * _point.x + b * _point.y + c * _point.z + d );
    }
    
    float UnsignedDistanceX( const glm::vec3& _point ) const
    {
        return fabsf( a * _point.x + c * _point.z + d );
    }

    glm::vec3 ClosestPoint( const glm::vec3& _point ) const
    {
        return glm::vec3( _point.x - a, _point.y - b, _point.z - c ) * SignedDistance( _point );
    }

    glm::vec3 ProjectPoint( const glm::vec3& _point ) const
    {
        return _point - Normal() * SignedDistance( _point );
    }
    
    glm::vec3 ProjectPointX( const glm::vec3& _point ) const
    {
        return _point - glm::vec3( -1.0f, 0.0f, 0.0f ) * SignedDistance( _point );
    }

    glm::vec3 ProjectPointY( const glm::vec3& _point ) const
    {
        return _point - glm::vec3( 0.0f, -1.0f, 0.0f ) * SignedDistance( _point );
    }

    bool IntersectSphere( const glm::vec3& _center, float _radius, glm::vec3 * _circleMidPoint, float * _circleRadius ) const
    {
        // http://www.ambrsoft.com/TrigoCalc/Sphere/SpherePlaneIntersection_.htm
        // Note: expects plane normal to be normalized (to save on the normalizing of the distToSphereCenter)

        float num = a * _center.x + b * _center.y + c * _center.z + d;
        float distToSphereCenter = fabsf( num );

        if ( distToSphereCenter > _radius )
        {
            return false;
        }

        float abc2 = a*a + b*b + c*c;
        float rcpAbc2 = 1.0 / abc2;        
        
        _circleMidPoint->x = _center.x - ( a * num ) * rcpAbc2;
        _circleMidPoint->y = _center.y - ( b * num ) * rcpAbc2;
        _circleMidPoint->z = _center.z - ( c * num ) * rcpAbc2;

      //  ci::app::console() << "RADIUS: " << _radius << ", distToSphereCenter: " << distToSphereCenter << std::endl;

        *_circleRadius = sqrt( _radius * _radius - distToSphereCenter * distToSphereCenter );
        return true;
    }
    
    // Optimized form for a Y plane
    bool IntersectSphereY( const glm::vec3& _center, float _radius, glm::vec3 * _circleMidPoint, float * _circleRadius ) const
    {
        // http://www.ambrsoft.com/TrigoCalc/Sphere/SpherePlaneIntersection_.htm
        // Note: expects plane normal to be normalized (to save on the normalizing of the distToSphereCenter)

        float num = b * _center.y + c * _center.z + d;
        float distToSphereCenter = fabsf( num );

        if ( distToSphereCenter > _radius )
        {
            return false;
        }

        float abc2 = b*b + c*c;
        float rcpAbc2 = 1.0 / abc2;        
        
        _circleMidPoint->x = _center.x;
        _circleMidPoint->y = _center.y - ( b * num ) * rcpAbc2;
        _circleMidPoint->z = _center.z - ( c * num ) * rcpAbc2;

        *_circleRadius = sqrt( _radius * _radius - distToSphereCenter * distToSphereCenter );
        return true;
    }

    // Optimized form for a Z plane
    bool IntersectSphereZ( const glm::vec3& _center, float _radius, glm::vec3 * _circleMidPoint, float * _circleRadius ) const
    {
        // http://www.ambrsoft.com/TrigoCalc/Sphere/SpherePlaneIntersection_.htm
        // Note: expects plane normal to be normalized (to save on the normalizing of the distToSphereCenter)

        float num =  c * _center.z + d;
        float distToSphereCenter = fabsf( num );

        if ( distToSphereCenter > _radius )
        {
            return false;
        }

        float rcpC2 = 1.0 / ( c * c );        
        
        _circleMidPoint->x = _center.x;
        _circleMidPoint->y = _center.y;
        _circleMidPoint->z = _center.z - ( c * num ) * rcpC2;

        *_circleRadius = sqrt( _radius * _radius - distToSphereCenter * distToSphereCenter );
        return true;
    }

    void Flip()
    {
        a = -a;
        b = -b;
        c = -c;
        d = -d;
    }

    void FlipNormal()
    {
        a = -a;
        b = -b;
        c = -c;
    }

    float   a;
    float   b;
    float   c;
    float   d;
};

}

