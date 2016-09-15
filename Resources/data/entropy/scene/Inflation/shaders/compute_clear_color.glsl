#version 450

struct BlobVertex{
	vec4 pos;
	vec4 vel;
};

layout (std140, binding=0) buffer blob_positions
{
	BlobVertex vertices[];
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;
void main(void)
{
	uint idx = gl_GlobalInvocationID.x;
	vertices[idx].vel = vec4(0);
}
