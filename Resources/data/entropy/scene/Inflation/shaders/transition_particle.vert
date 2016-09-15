#version 450

struct BlobVertex{
	vec4 pos;
	vec4 col;
};

layout (std140, binding=0) buffer blob_positions
{
	BlobVertex vertices[];
};

uniform float every;
uniform float scale;
uniform mat4 modelViewProjectionMatrix;

in vec3 position;

void main(void)
{
	vec3 model_position = vertices[(gl_InstanceID + int(every))*int(every)].pos.xyz;
	gl_Position = modelViewProjectionMatrix * vec4(position * scale + model_position, 1.0);
}
