#version 330

in vec4 position;
in vec4 color;

uniform mat4 modelViewProjectionMatrix;
uniform float alpha;

out vec4 v_color;


void main(void)
{
	gl_Position = modelViewProjectionMatrix * position;
	v_color = vec4(color.rgb, color.a * alpha);
}
