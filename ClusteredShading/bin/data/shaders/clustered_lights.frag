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

#define saturate(x) clamp(x, 0.0, 1.0)

uniform usampler3D     uLightPointerTex;
uniform usamplerBuffer uLightIndexTex;
//uniform usampler1D   uLightIndexTex;
//uniform mat4           uCamViewMat;

uniform float          uNearClip;
uniform float          uFarClip;
uniform float          uNumClusters;

//uniform	sampler2D	   uNormalMap;

struct PointLight
{
    vec4  position;
    vec4  color;
    float radius;
    float intensity;
};

layout (std140) uniform PointLightBlock
{
    PointLight PointLights[ 1024 ];
};

in vec4 vVertex;
in vec3 vNormal;
in vec2 vTexCoord0;

out vec4 oColor;

const float NUM_SLICES_Z = 16.0f;

// Attenuation method described in Epic's UE4 paper
float CalcPointLightAttenuation( vec3 lightPosition, vec3 vertexPosition, float lightRadius )
{
    float distToLight = length( vertexPosition - lightPosition );
    float falloff = pow( saturate( 1.0f - pow( distToLight / lightRadius, 4 ) ), 2 ) / ( distToLight * distToLight + 1.0f );
    return falloff;
}

ivec3 getClusterCoord( vec2 coord_xy, float linearDepth )
{
    int slice = int( max( linearDepth * NUM_SLICES_Z, 0.0f ) );

    // tile size is 64, SHR by 6 to find x,y index
    ivec3 cluster_coord;
    cluster_coord.x = int( coord_xy.x / 960.0f * 20 );// >> 6;
    cluster_coord.y = int( coord_xy.y / 720.0f * 11 );// >> 6;
    cluster_coord.z = slice;

    return cluster_coord;
}

void main( void )
{
    float linearDepth = ( -vVertex.z - uNearClip ) / ( uFarClip - uNearClip );

    ivec3 cluster_coord = getClusterCoord( gl_FragCoord.xy, linearDepth );
    uvec4 light_data = texelFetch( uLightPointerTex, cluster_coord, 0 );

    int light_index_offset = int( light_data.x );
    int point_light_count = int( light_data.y );

    vec4 col = vec4( 0.0f );

    vec3 N = normalize( vNormal );

    for ( int i = 0; i < point_light_count; ++i )
    {
        uint index = texelFetch( uLightIndexTex, light_index_offset++ ).r; 

        PointLight pl = PointLights[ index ];
        vec3 lightPosVS = ( viewMatrix * pl.position ).xyz;
        vec3 lightDir = lightPosVS - vVertex.xyz;

        float dist = length( lightDir );
        lightDir = normalize( lightDir );

        float lambert = clamp( dot( lightDir, N ), 0.0f, 1.0f );

        float attenuation = CalcPointLightAttenuation( lightPosVS, vVertex.xyz, pl.radius );
        col.rgb += pl.color.rgb * lambert * attenuation * pl.intensity; 
    }

//     col.rgb = vec3( ( float( light_index_offset ) + 1.0f ) / uNumClusters );
 //      col.rgb = vec3( float(cluster_coord.x) / 20.0f, float(cluster_coord.y) / 11.0f, float( slice ) / NUM_SLICES_Z );

    oColor = vec4( col.rgb, 1.0f );
   // oColor = vec4( N, 1.0f );
   // oColor = vec4( vVertex.xyz, 1.0f );
}
