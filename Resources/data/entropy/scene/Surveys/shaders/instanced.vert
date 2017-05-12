#version 440

uniform mat4 modelViewProjectionMatrix;

struct InstanceData
{
	mat4 transform;
	float alpha;
	float starFormationRate;
	vec2 dummy;
};

layout(std140, binding=0) buffer instanceData
{
	InstanceData uData[];
};

in vec4 position;
in vec4 color;

out vec4 vColor;
out float vAlpha;

void main()
{
	mat4 transformMatrix = uData[gl_InstanceID].transform;
	gl_Position = modelViewProjectionMatrix * transformMatrix * position;

	vColor = color;
	vAlpha = uData[gl_InstanceID].alpha;
}
