#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

in float vEnabled;

out vec4 fragColor;

void main(void)
{
	if (vEnabled < 0.5) discard;

	fragColor = texture(uTex0, gl_PointCoord) * globalColor;
}
