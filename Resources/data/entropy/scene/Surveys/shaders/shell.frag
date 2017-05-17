#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

flat in int vEnabled;

out vec4 fragColor;

void main(void)
{
	if (vEnabled == 0) discard;

	vec2 texCoord = gl_PointCoord;

	fragColor = vec4((texture(uTex0, texCoord) * globalColor).rgb, 0.85);
}
