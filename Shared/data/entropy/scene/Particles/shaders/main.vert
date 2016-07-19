// Filename: main.vert
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

#define USE_INSTANCED 1

#ifdef USE_INSTANCED
uniform samplerBuffer uOffsetTex;
#else
uniform mat4 uNormalMatrix;
#endif

// View space.
out vec4 vVertex_vs;
out vec3 vNormal_vs;

// World space.
out vec3 vVertex_ws;
out vec3 vEyeDir_ws;
out vec3 vNormal_ws;

out vec2 vTexCoord0;

void main( void ) 
{
#ifdef USE_INSTANCED
	int idx = gl_InstanceID * 4;
	mat4 modelMatrix = mat4(
		texelFetch( uOffsetTex, idx+0 ),
		texelFetch( uOffsetTex, idx+1 ),
		texelFetch( uOffsetTex, idx+2 ), 
		texelFetch( uOffsetTex, idx+3 )
	);

	vec4 vPosition_ws = modelMatrix * position;
	vVertex_vs = modelViewMatrix * vPosition_ws;
	vVertex_ws = vPosition_ws.xyz;
	
	mat4 normalMatrix = transpose( inverse( modelViewMatrix * modelMatrix ) );
	vNormal_vs = normalize( ( normalMatrix * vec4( normal, 0.0 ) ).xyz );
	vNormal_ws = ( viewData.inverseViewMatrix * vec4( vNormal_vs, 0.0 ) ).xyz;

	vec4 eyeDir_vs = vVertex_vs - vec4( 0.0, 0.0, 0.0, 1.0 );
	vEyeDir_ws = ( viewData.inverseViewMatrix * eyeDir_vs ).xyz;
#else	
	vVertex_vs = modelViewMatrix * position;
	vVertex_ws = ( viewData.inverseViewMatrix * vVertex ).xyz;

	vNormal_vs = normalize( ( uNormalMatrix * vec4( normal, 0.0 ) ).xyz );
	vNormal_ws = ( viewData.inverseViewMatrix * vec4( vNormal, 0.0 ) ).xyz;

	vec4 eyeDir_vs = vVertex_vs - vec4( 0.0, 0.0, 0.0, 1.0 );
	vEyeDir_ws = ( viewData.inverseViewMatrix * eyeDir_vs ).xyz;
#endif

	vTexCoord0 = texcoord;

	gl_Position = projectionMatrix * vVertex_vs;
}
