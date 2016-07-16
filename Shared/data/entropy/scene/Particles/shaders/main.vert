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

uniform mat4 uNormalMatrix;

out vec4 vVertex;
out vec3 vNormal;
out vec2 vTexCoord0;

// Cube map world space
out vec3 vVertex_ws;
out vec3 vEyeDir_ws;
out vec3 vNormal_ws;

void main( void )
{
    vVertex = modelViewMatrix * position; // calculate view space position (required for lighting)
	vNormal = normalize((uNormalMatrix * vec4(normal, 0.0)).xyz); // calculate view space normal (required for lighting & normal mapping)
    vTexCoord0 = texcoord; // pass texture coordinates

    // Cube map vectors
    vec4 eyeDir_vs = vVertex - vec4( 0.0, 0.0, 0.0, 1.0);
    vVertex_ws = (viewData.inverseViewMatrix * vVertex).xyz;
    vEyeDir_ws = vec3(viewData.inverseViewMatrix * eyeDir_vs);
    vNormal_ws = vec3(viewData.inverseViewMatrix * vec4( vNormal, 0.0));

    gl_Position = projectionMatrix * vVertex;
}
