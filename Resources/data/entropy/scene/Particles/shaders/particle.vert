// Filename: main.vert
// 
// Copyright © James Acres
#version 400

#pragma include <inc/ofDefaultUniforms.glsl>
#pragma include <inc/ofDefaultVertexInAttributes.glsl> 

#pragma include <inc/viewData.glsl>

uniform samplerBuffer uOffsetTex;
uniform float uScale;

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
	int idx = gl_InstanceID * 4;
	mat4 modelMatrix = mat4(
		texelFetch( uOffsetTex, idx+0 ),
		texelFetch( uOffsetTex, idx+1 ),
		texelFetch( uOffsetTex, idx+2 ), 
		texelFetch( uOffsetTex, idx+3 )
	);

	vec4 vPosition_ws = modelMatrix * ( position * vec4( uScale, uScale, uScale, 1.0 ) );
	vVertex_vs = modelViewMatrix * vPosition_ws;
	vVertex_ws = vPosition_ws.xyz;
	
	mat4 normalMatrix = transpose( inverse( modelViewMatrix * modelMatrix ) );
	vNormal_vs = normalize( ( normalMatrix * vec4( normal, 0.0 ) ).xyz );
	vNormal_ws = ( viewData.inverseViewMatrix * vec4( vNormal_vs, 0.0 ) ).xyz;

	vec4 eyeDir_vs = vVertex_vs - vec4( 0.0, 0.0, 0.0, 1.0 );
	vEyeDir_ws = ( viewData.inverseViewMatrix * eyeDir_vs ).xyz;

	vTexCoord0 = texcoord;

	gl_Position = projectionMatrix * vVertex_vs;
}
