// Filename: cube_map.vert
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Tue Nov 10 11:28:09 2015 (-0500)
// Last-Updated: Wed Nov 11 13:46:31 2015 (-0500)

#version 330

#pragma include <inc/ofDefaultUniforms.glsl>

#pragma include <inc/viewData.glsl>

smooth out vec3 vEyeDir;

const vec2 triangleVerts[3] = vec2[](
  vec2(-1, -1),
  vec2( 3, -1),
  vec2(-1,  3)
);

void main( void )
{
    vec4 vertex = vec4(triangleVerts[gl_VertexID], 0.0f, 1.0f);
    vec3 unprojected = (inverse(projectionMatrix) * vertex).xyz;
    vEyeDir = mat3(viewData.inverseViewMatrix) * unprojected;

    gl_Position = vec4(vertex.xyz, 1.0f);
}
