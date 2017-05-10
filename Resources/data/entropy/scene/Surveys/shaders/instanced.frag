#version 150

uniform vec4 globalColor;

in vec4 vColor;
in float vAlpha;

out vec4 fragColor;

void main(void)
{
	// Additive blending, so using alpha as a scalar.
	fragColor = globalColor * vColor * vAlpha;
}
