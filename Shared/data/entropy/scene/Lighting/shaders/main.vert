// Filename: passthrough.vert
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Tue Nov 10 21:26:11 2015 (-0500)
// Last-Updated: Mon Jan 25 10:15:51 2016 (-0500)

#version 400

#pragma include <inc/ofDefaultUniforms.glsl>
#pragma include <inc/ofDefaultVertexInAttributes.glsl> 

#pragma include <inc/viewData.glsl>

//#define USE_INSTANCED 1

#ifdef USE_INSTANCED
uniform samplerBuffer uOffsetTex;
#else
uniform mat4 uNormalMatrix;
#endif

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

// Cube map world space
out vec3 vPosition_ws;
out vec3 vEyeDir_ws;
out vec3 vNormal_ws;

void main( void )
{
#ifdef USE_INSTANCED
    int idx = gl_InstanceID * 4;
    mat4 transform = mat4( 
        texelFetch(uOffsetTex, idx+0),
        texelFetch(uOffsetTex, idx+1),
        texelFetch(uOffsetTex, idx+2), 
        texelFetch(uOffsetTex, idx+3)
    );
    mat4 transformMatrix = modelViewMatrix * transform;
	mat4 normalMatrix = transpose(inverse(transformMatrix));
    
    vPosition = transformMatrix * position;
    vNormal = normalize((normalMatrix * vec4(normal, 0.0)).xyz);

    // Cube map vectors
	mat4 inverseTransformMatrix = inverse(transformMatrix);
    vec4 eyeDir_vs = vPosition - vec4( 0.0, 0.0, 0.0, 1.0);
    vPosition_ws = (inverseTransformMatrix * vPosition).xyz;
    vEyeDir_ws = (inverseTransformMatrix * eyeDir_vs).xyz;
    vNormal_ws = (inverseTransformMatrix * vec4(vNormal, 0.0)).xyz;
#else	
	vPosition = modelViewMatrix * position; // calculate view space position (required for lighting)
	vNormal = normalize((uNormalMatrix * vec4(normal, 0.0)).xyz); // calculate view space normal (required for lighting & normal mapping)

    // Cube map vectors
    vec4 eyeDir_vs = vPosition - vec4( 0.0, 0.0, 0.0, 1.0);
    vPosition_ws = (viewData.inverseViewMatrix * vPosition).xyz;
    vEyeDir_ws = (viewData.inverseViewMatrix * eyeDir_vs).xyz;
    vNormal_ws = (viewData.inverseViewMatrix * vec4(vNormal, 0.0)).xyz;
#endif

    vTexCoord = texcoord; // pass texture coordinates

    gl_Position = projectionMatrix * vPosition;
}
