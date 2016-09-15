#version 450

uniform vec4 particleColor;
out vec4 fragColor;

void main(void)
{
	fragColor = particleColor;
}
