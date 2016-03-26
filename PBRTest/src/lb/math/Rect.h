#pragma once

#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include <math.h>

namespace lb
{
    struct Rect
    {
        Rect()
            : x( 0 )
            , y( 0 )
            , w( 0 )
            , h( 0 )
        {};

        Rect( float _x, float _y, float _w, float _h )
            : x( _x )
            , y( _y )
            , w( _w )
            , h( _h )
        {};

        float x;
        float y;
        float w;
        float h;
    };
}