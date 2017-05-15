#version 150

// OF default uniforms and attributes
uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in vec2 texcoord;

// App uniforms and attributes
uniform float uMaxAge;

in float age;

out float vAlpha;

void main()
{
	vAlpha = age / uMaxAge;

	gl_Position = modelViewProjectionMatrix * position;
}
