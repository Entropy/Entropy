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
uniform vec4 particleColor;
uniform mat4 modelViewProjectionMatrix;

in vec3 position;

out vec4 v_position;
out vec4 v_color;

void main(void)
{
	vec3 model_position = vertices[(gl_InstanceID + int(every)) * int(every)].pos.xyz;
	v_position = vec4(position * scale + model_position, 1.0);
	v_color = particleColor;
}
