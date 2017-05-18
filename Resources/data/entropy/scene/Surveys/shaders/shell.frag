#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

out vec4 fragColor;

void main(void)
{
	vec2 texCoord = gl_PointCoord;

	fragColor = vec4((texture(uTex0, texCoord) * globalColor).rgb, 0.85);
}
