#version 330

in vec4 position;
in vec2 texcoord;
out vec2 v_texcoord;
uniform mat4 modelViewProjectionMatrix;

void main(void)
{
	gl_Position = modelViewProjectionMatrix * position;
	v_texcoord = texcoord;
}
