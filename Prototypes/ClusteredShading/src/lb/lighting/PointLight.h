#pragma once

#include "glm/glm.hpp"

namespace lb
{
    struct PointLight
    {
        PointLight() 
        : position( glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f ) )
        , color( glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) )
        , radius( 1.0f )
        , intensity( 500.0f )
        {};

        PointLight( const glm::vec3& _pos, const glm::vec3& _color, float _radius, float _intensity )
            : position( _pos.x, _pos.y, _pos.z, 1.0f )
            , color( _color.x, _color.y, _color.z, 1.0f )
            , radius( _radius )
            , intensity( _intensity )
        {
        }

        glm::vec4   position;
        glm::vec4   color;
        float       radius;
        float       intensity;
        float       padding[ 2 ]; // align to vec4 for UBO
    };
}

