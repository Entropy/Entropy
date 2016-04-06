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

#pragma include <inc/of_default_uniforms.glsl>
#pragma include <inc/of_default_vertex_in_attributes.glsl> 

#pragma include <inc/view_info.glsl>

uniform mat3 normalMatrix;

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
    vNormal = normalize(normalMatrix * normal); // calculate view space normal (required for lighting & normal mapping)
    vTexCoord0 = texcoord; // pass texture coordinates

    // Cube map vectors
    vec4 eyeDir_vs = vVertex - vec4( 0.0, 0.0, 0.0, 1.0 );
    vVertex_ws = ( viewInfo.viewMatrixInverse * vVertex).xyz;
    vEyeDir_ws = vec3( viewInfo.viewMatrixInverse * eyeDir_vs );
    vNormal_ws = vec3( viewInfo.viewMatrixInverse * vec4( vNormal, 0.0 ) );

    gl_Position = projectionMatrix * vVertex;
}
