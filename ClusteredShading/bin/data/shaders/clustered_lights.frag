// Filename: debugNormals.frag
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov 14 13:50:14 2015 (-0500)
// Last-Updated: Mon Jan 25 17:05:45 2016 (-0500)

#version 330 core

#pragma include <of_default_uniforms.glsl>

#pragma include <projection_info.glsl>
#pragma include <clustered_shading.glsl>

#define saturate(x) clamp(x, 0.0, 1.0)

in vec4 vVertex;
in vec3 vNormal;
in vec2 vTexCoord0;

out vec4 oColor;


// Attenuation method described in Epic's UE4 paper
float CalcPointLightAttenuation( vec3 lightPosition, vec3 vertexPosition, float lightRadius )
{
    float distToLight = length( vertexPosition - lightPosition );
    float falloff = pow( saturate( 1.0f - pow( distToLight / lightRadius, 4 ) ), 2 ) / ( distToLight * distToLight + 1.0f );
    return falloff;
}



void main( void )
{
    vec4 col = vec4( 0.0f );
    vec3 N = normalize( vNormal );

    int lightIndexOffset = 0;
    int pointLightCount = 0;
    GetLightOffsetAndCount( gl_FragCoord.xy, vVertex.z, lightIndexOffset, pointLightCount );

    for ( int i = 0; i < pointLightCount; ++i )
    {
        PointLight pl = GetPointLight( lightIndexOffset++ );
        vec3 lightPosVS = ( viewMatrix * pl.position ).xyz;
        vec3 lightDir = lightPosVS - vVertex.xyz;

        float dist = length( lightDir );
        lightDir = normalize( lightDir );

        float lambert = clamp( dot( lightDir, N ), 0.0f, 1.0f );

        float attenuation = CalcPointLightAttenuation( lightPosVS, vVertex.xyz, pl.radius );
        col.rgb += pl.color.rgb * lambert * attenuation * pl.intensity; 
    }

    oColor = vec4( col.rgb, 1.0f );
}
