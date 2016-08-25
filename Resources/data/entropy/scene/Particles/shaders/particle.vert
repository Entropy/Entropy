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
#define NUM_PARTICLE_TYPES 9
uniform vec4 colors[NUM_PARTICLE_TYPES];

/*const vec4 colors[5] = {
	{0.5,0.5,0.5,0.5},
	{0.0,0.1,0.5,0.5},
	{0.2,0.2,0.2,0.5},
	{0.1,0.1,0.1,0.5},
	{0.0,0.1,0.5,0.5},
};*/

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
	out_color = colors[int(uType) % NUM_PARTICLE_TYPES];
#else
	out_color = colors[(int(uType) + gl_VertexID) % NUM_PARTICLE_TYPES];
#endif
	out_normal = vec4(mat3(modelMatrix) * normal, 1.0);
}
