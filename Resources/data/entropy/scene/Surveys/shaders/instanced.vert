#version 440

uniform mat4 modelViewProjectionMatrix;

struct InstanceData
{
	mat4 transform;
	float alpha;
	float starFormationRate;
	uint densityMod;
	float dummy;
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

	if (gl_VertexID % uData[gl_InstanceID].densityMod != 0)
	{
		gl_Position.w = 0;
	}

	vColor = color;
	vAlpha = uData[gl_InstanceID].alpha;
}
