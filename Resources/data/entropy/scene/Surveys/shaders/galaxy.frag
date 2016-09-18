#version 150

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
	fragColor = vec4((globalColor * texColor).rgb, val);
	//fragColor = vec4(vec3(val), 1.0);
}
