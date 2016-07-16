// Filename: cube_map.frag
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Tue Nov 10 11:32:11 2015 (-0500)
// Last-Updated: Mon Nov 23 11:30:47 2015 (-0500)

#version 330

#pragma include <inc/toneMapping.glsl>

uniform samplerCube uCubeMap;

smooth in vec3 oEyeDirection;

out vec4 oColor;

void main( void )
{
    vec3 color = texture( uCubeMap, oEyeDirection ).rgb;

    // tonemap function requires exposure corrected color
    color = tonemapUncharted2( color * uExposure );

    // Correct for white input level
    const float whiteInputLevel = 2.0f;
    vec3 whiteScale = 1.0f / tonemapUncharted2( vec3( whiteInputLevel ) );
    color = color * whiteScale;

    oColor = vec4( linearToGamma( color, uGamma ), 1.0f );
    //oColor = vec4( color, 1.0f );
}
