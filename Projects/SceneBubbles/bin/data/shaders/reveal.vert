#version 150

// OF default uniforms and attributes
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

in vec4 position;
in vec2 texcoord;

// App uniforms and attributes
out vec4 vPosition_ws;
out vec2 vTexCoord;

void main()
{
	vPosition_ws = modelViewMatrix * position;
	vTexCoord = texcoord;

	gl_Position = projectionMatrix * vPosition_ws;
}
