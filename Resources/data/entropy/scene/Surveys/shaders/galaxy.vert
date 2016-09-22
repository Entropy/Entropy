#version 150

uniform mat4 viewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform mat4 uNormalMatrix;

in vec4 position;
in vec2 texcoord;
in vec3 normal;

out vec3 vNormal;
out vec2 vTexCoord;

void main()
{
	vec4 normal_ws = uNormalMatrix * vec4(normal, 0.0);
	vNormal = (normal_ws).xyz;

	vTexCoord = texcoord;

	gl_Position = modelViewProjectionMatrix * position;
}
