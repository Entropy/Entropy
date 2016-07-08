#pragma once

#include "glm/glm.hpp"

namespace lb
{
    struct DirectionalLight
    {
        DirectionalLight() 
        : direction( glm::vec4( 1.0f, 0.0f, 0.0f, 1.0f ) )
        , zero( 0.0f )
        , color( glm::vec3( 1.0f, 1.0f, 1.0f ) )
        , intensity( 1000.0f )
        {};

        DirectionalLight( const glm::vec3& _direction, const glm::vec3& _color, float _intensity )
            : direction( _direction )
            , zero( 0.0f )
            , color( _color )
            , intensity( _intensity )
        {
        }

    public:
        glm::vec3   direction;

    private:
        float       zero; // .w = 0 for point lights

    public:
        glm::vec3   color;
        float       intensity;
    };
}

