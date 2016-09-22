#version 150

#define BLEND_ALPHA 0

uniform vec4 globalColor;

uniform sampler2D uTex0;
uniform float uAlphaBase;

in vec3 vNormal;
in vec2 vTexCoord;

out vec4 fragColor;

void main(void)
{
	float val = dot(normalize(vNormal), vec3(0.0, 0.0, -1.0));
	val = clamp(val, 0.0, 1.0);
	//val = val * val;
	vec4 texColor = texture(uTex0, vTexCoord);
#if BLEND_ALPHA
	fragColor = vec4((globalColor * texColor).rgb, val * uAlphaBase);
#else
	fragColor = vec4((globalColor * texColor).rgb * val * uAlphaBase, 1.0);
#endif
}
