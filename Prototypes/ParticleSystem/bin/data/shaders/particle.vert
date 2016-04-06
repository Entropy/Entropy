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

uniform samplerBuffer uOffsetTex;

out vec4 vVertex;
out vec3 vNormal;

// Cube map world space
out vec3 vVertex_ws;
out vec3 vEyeDir_ws;
out vec3 vNormal_ws;


 mat4 createTransformMatrix( float _yAngle )
{
    float c = cos(_yAngle);
    float s = sin(_yAngle);
    float t = 1.0 - c;

    return mat4(
      c,    0.0,   -s,    0.0,
      0.0,  t + c,  0.0,  0.0,
      s,    0.0,    c,    0.0,
      0.0,  0.0,    0.0,  1.0
    );
}

#define scale offset.w

void main( void )
{
/*
    int idx = gl_InstanceID * 2;
    vec4 offset = texelFetch( uOffsetTex, idx );
    vec4 rotation = texelFetch( uOffsetTex, idx + 1 );

    float cx = cos( rotation.x );
    float sx = sin( rotation.x );

    float cy = cos( rotation.y );
    float sy = sin( rotation.y );

    float cz = cos( rotation.z );
    float sz = sin( rotation.z );

    mat4 transform = mat4(
        -scale, 0.0f, 0.0f, 0.0f,
        0.0f, -scale, 0.0f, 0.0f,
        0.0f, 0.0f, -scale, 0.0f,
        offset.x, offset.y, offset.z, 1.0f
    );
*/

    int idx = gl_InstanceID * 4;
    mat4 transform = mat4( 
        texelFetch( uOffsetTex, idx ),
        texelFetch( uOffsetTex, idx+1 ),
        texelFetch( uOffsetTex, idx+2 ), 
        texelFetch( uOffsetTex, idx+3 )
    );

    mat4 mvMatrix = modelViewMatrix * transform;

    vVertex = mvMatrix * position; // calculate view space position (required for lighting)
    vNormal = normalize( mat3( mvMatrix ) * normal ); // calculate view space normal (required for lighting & normal mapping)

    // Cube map vectors
    vec4 eyeDir_vs = vVertex - vec4( 0.0, 0.0, 0.0, 1.0 );
    vVertex_ws = ( viewInfo.viewMatrixInverse * vVertex ).xyz;
    vEyeDir_ws = vec3( viewInfo.viewMatrixInverse * eyeDir_vs );
    vNormal_ws = vec3( viewInfo.viewMatrixInverse * vec4( vNormal, 0.0 ) );

    gl_Position = projectionMatrix * vVertex;
}
