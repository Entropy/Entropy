#pragma once
#include "glm/glm.hpp"
#include "lb/math/Plane.h"

namespace lb
{
bool SphereInFrustum( const lb::Plane* _planes, const glm::vec3& _center, float _radius )
{
    bool bOutside = false;
    bOutside |= _planes[ 0 ].SignedDistance( _center ) < -_radius;
    bOutside |= _planes[ 1 ].SignedDistance( _center ) > _radius; // TODO(James): Fix this
    bOutside |= _planes[ 2 ].SignedDistance( _center ) < -_radius;
    bOutside |= _planes[ 3 ].SignedDistance( _center ) < -_radius;
    bOutside |= _planes[ 4 ].SignedDistance( _center ) < -_radius;
    bOutside |= _planes[ 5 ].SignedDistance( _center ) < -_radius;

    return !bOutside;
}

bool BoxInFrustum( const lb::Plane* _planes, const glm::vec3& _aabbMin, const glm::vec3& _aabbMax )
{
    // check all corners of AABB against frustum planes
    for ( int i = 0; i < 6; ++i )
    {
        int out = 0;
        const glm::vec3& normal = _planes[ i ].Normal();
        out += ( glm::dot( normal, glm::vec3( _aabbMin.x, _aabbMin.y, _aabbMin.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMax.x, _aabbMin.y, _aabbMin.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMin.x, _aabbMax.y, _aabbMin.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMax.x, _aabbMax.y, _aabbMin.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMin.x, _aabbMin.y, _aabbMax.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMax.x, _aabbMin.y, _aabbMax.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMin.x, _aabbMax.y, _aabbMax.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;
        out += ( glm::dot( normal, glm::vec3( _aabbMax.x, _aabbMax.y, _aabbMax.z ) ) < 0.0f ) ? 1 : 0;
        //console() << "i: " << i << " out: " << out << endl;

        if ( out == 8 )
        {
            return false;
        }
    }

    return true;
}
}