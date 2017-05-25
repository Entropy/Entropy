#version 450
#extension GL_EXT_shader_image_load_store : enable

coherent restrict layout(r32ui, binding=0) uniform readonly uimage3D ivolume;
coherent restrict layout(r16f, binding=1) uniform writeonly image3D volume;
uniform float uDensityMin;
uniform float uDensityMax;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	uvec4 d = imageLoad(ivolume, ivec3(gl_GlobalInvocationID));
	imageStore(volume, ivec3(gl_GlobalInvocationID), vec4(d.r/pow(2,32)));
}
