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

#pragma include <of_default_uniforms.glsl>
#pragma include <of_default_vertex_in_attributes.glsl> 

uniform mat3    normalMatrix;

/*uniform mat4 uCamProjMat;
uniform mat4 uCamViewMat;
uniform mat3 uCamNormalMat;
*/

out vec4 vVertex;
out vec3 vNormal;
out vec2 vTexCoord0;

void main( void )
{
    // calculate view space position (required for lighting)
    vVertex = modelViewMatrix * position; 

    // calculate view space normal (required for lighting & normal mapping)
    vNormal = normalize(normalMatrix * normal);

    // calculate tangent and construct the bitangent (required for normal mapping)
    //    vTangent = normalize(ciNormalMatrix * tangent); 
    //   vBiTangent = normalize(cross(vNormal, vTangent));

    // pass texture coordinates
    vTexCoord0 = texcoord;

    gl_Position = projectionMatrix * vVertex;
}
