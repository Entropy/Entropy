// Filename: main.vert
//
// Copyright ? James Acres
#version 450

#pragma include <inc/ofDefaultUniforms.glsl>
#pragma include <inc/ofDefaultVertexInAttributes.glsl>

uniform samplerBuffer uOffsetTex;
uniform float uScale;
uniform float uType;

out vec4 out_position;
out vec4 out_color;
out vec4 out_normal;

#define COLOR_PER_TYPE 1

const vec4 colors[5] = {
    {0.5,0.5,0.5,0.5},
    {0.0,0.1,0.5,0.5},
    {0.2,0.2,0.2,0.5},
    {0.1,0.1,0.1,0.5},
    {0.0,0.1,0.5,0.5},
};

void main( void )
{
	int idx = gl_InstanceID * 4;
	mat4 modelMatrix = mat4(
	    texelFetch( uOffsetTex, idx+0 ),
	    texelFetch( uOffsetTex, idx+1 ),
	    texelFetch( uOffsetTex, idx+2 ),
	    texelFetch( uOffsetTex, idx+3 )
	);
	out_position = modelMatrix * ( position * vec4( uScale, uScale, uScale, 1.0 ) );
#if COLOR_PER_TYPE
	out_color = colors[int(uType) % 5];
#else
	out_color = colors[(int(uType) + gl_VertexID) % 5];
#endif
	out_normal = vec4(mat3(modelMatrix) * normal, 1.0);
}
