// Filename: SphereScissor.h
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Fri Jan  8 17:11:10 2016 (-0500)
// Last-Updated: Sat Jan  9 11:57:01 2016 (-0500)

#pragma once

#include "glm/glm.hpp"
#include "glm/vec3.hpp"
#include <math.h>

#include "lb/math/Rect.h"

namespace lb
{
    enum class SphereProjectionResult
    {
        EMPTY,
        PARTIAL,
        FULL
    };

    SphereProjectionResult ProjectSphere( const glm::vec3& _center, float _radius, float _focalLength, float _aspectRatio, glm::vec2 *rectMin, glm::vec2 *rectMax ) 
    {
//        float focalLength = 1.0f / ( tanf( toRadians( m_camera.getFovHorizontal() * 0.5f ) * 1.0f ) );
//        float aspectRatio = getWindowHeight() / (float)getWindowWidth();
        
        float cx = _center.x;
        float cy = _center.y;
        float cz = _center.z;
        float r2 = _radius * _radius;
     
        float cx2 = cx * cx;
        float cy2 = cy * cy;
        float cz2 = cz * cz;
        float cxz2 = cx2 + cz2;

        if (cxz2 + cy2 > r2)
        {
            float left = -1.0F;
            float right = 1.0F;
            float bottom = -1.0F;
            float top = 1.0F;
     
            float rcz = 1.0F / cz;
     
            float dx = r2 * cx2 - cxz2 * (r2 - cz2);
            if (dx > 0.0F)
            {
                dx = sqrtf(dx);
                float ax = 1.0F / cxz2;
                float bx = _radius * cx;
     
                float nx1 = (bx + dx) * ax;
                float nx2 = (bx - dx) * ax;
     
                float nz1 = (_radius - nx1 * cx) * rcz;
                float nz2 = (_radius - nx2 * cx) * rcz;
     
                float pz1 = cz - _radius * nz1;
                float pz2 = cz - _radius * nz2;
     
                if (pz1 < 0.0F)
                {
                    float x = nz1 * _focalLength / nx1;
                    if (nx1 > 0.0F) left = fmaxf(left, x);
                    else right = fminf(right, x);
                }
     
                if (pz2 < 0.0F)
                {
                    float x = nz2 * _focalLength / nx2;
                    if (nx2 > 0.0F) left = fmaxf(left, x);
                    else right = fminf(right, x);
                }
            }
     
            float cyz2 = cy2 + cz2;
            float dy = r2 * cy2 - cyz2 * (r2 - cz2);
            if (dy > 0.0F)
            {
                dy = sqrtf(dy);
                float ay = 1.0F / cyz2;
                float by = _radius * cy;
     
                float ny1 = (by + dy) * ay;
                float ny2 = (by - dy) * ay;
     
                float nz1 = (_radius - ny1 * cy) * rcz;
                float nz2 = (_radius - ny2 * cy) * rcz;
     
                float pz1 = cz - _radius * nz1;
                float pz2 = cz - _radius * nz2;
     
                if (pz1 < 0.0F)
                {
                    float y = ( nz1 * _focalLength) / (ny1 * _aspectRatio);

                    if ( ny1 > 0.0F )
                    {
                        bottom = fmaxf( bottom, y );
                    }
                    else
                    {
                        top = fminf( top, y );
                    }
                }
     
                if (pz2 < 0.0F)
                {
                    float y = (nz2 * _focalLength )/ (ny2 * _aspectRatio);

                    if ( ny2 > 0.0F )
                    {
                        bottom = fmaxf( bottom, y );
                    }
                    else
                    {
                        top = fminf( top, y );
                    }
                }
            }
     
            // outside of x and y bounds
            if ( ( !( left < right ) ) && ( !( bottom < top ) ) )
            {
                return ( SphereProjectionResult::EMPTY );
            }

            rectMin->x = left;
            rectMax->x = right;
            rectMin->y = bottom;
            rectMax->y = top;
     
            return ( SphereProjectionResult::PARTIAL );
        }
     
        return ( SphereProjectionResult::FULL );
    }

    glm::vec2 ScreenCoordFromClipCoord( const glm::vec3& _clipCoord, const lb::Rect& _viewport )
    {
        float x = _viewport.x + ( _clipCoord.x + 1.0f ) * 0.5f * _viewport.w;
        float y = _viewport.y + ( _clipCoord.y + 1.0f ) * 0.5f * _viewport.h;

        return glm::vec2( x, y );
    }
    
    glm::vec2 ScreenCoordFromClipCoord( const glm::vec2& _clipCoord, const lb::Rect& _viewport )
    {
        float x = _viewport.x + ( _clipCoord.x + 1.0f ) * 0.5f * _viewport.w;
        float y = _viewport.y + ( _clipCoord.y + 1.0f ) * 0.5f * _viewport.h;

        return glm::vec2( x, y );
    }
}
