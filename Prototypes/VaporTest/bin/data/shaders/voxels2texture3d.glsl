#version 450
#extension GL_EXT_shader_image_load_store : enable

coherent layout(r16f, location = 0, binding = 0) uniform writeonly image3D volume;
layout(location = 1) uniform isamplerBuffer voxels;
uniform float numVoxels;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
	//if(gl_GlobalInvocationID.x>=int(numVoxels)) return;
	ivec4 voxel = texelFetch(voxels, int(gl_GlobalInvocationID.x));
	int z = voxel.x & 511;
	int y = (voxel.x >> 9) & 511;
	int x = (voxel.x >> 18) & 511;
	imageStore(volume, ivec3(x,y,z), vec4(voxel.y)/pow(2.,32.));
}
