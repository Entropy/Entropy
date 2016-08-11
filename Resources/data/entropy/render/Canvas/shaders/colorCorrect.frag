// Filename: main.frag
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov 14 13:50:14 2015 (-0500)
// Last-Updated: Mon Jan 25 17:05:45 2016 (-0500)

#version 330 core

#pragma include <inc/toneMapping.glsl>

uniform float uBrightness;
uniform float uContrast;
uniform sampler2D uTex0;

in vec2 vTexCoord0;

out vec4 fragColor;

void main( void )
{
	vec4 baseColor = texture( uTex0, vTexCoord0 );

	vec3 color = colorCorrect( baseColor.rgb, uExposure, uTonemapType, uGamma );

	// Apply contrast.
    //color = mix( vec3( 0.18 ), color, uContrast );

    // Apply brightness.
    //color = max( vec3( 0.0 ), color + vec3( uBrightness ) );
	const vec3 halfVec = vec3(0.5);
	color = (color - halfVec) * uContrast + 0.5 + uBrightness;
	
	fragColor = vec4( color, baseColor.a );
}
