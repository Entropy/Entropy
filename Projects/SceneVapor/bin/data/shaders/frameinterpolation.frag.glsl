#version 330

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform float pct;
uniform float alpha;
in vec2 v_texcoord;
out vec4 fragColor;

void main()
{
	vec4 c2 = texture(texture2, v_texcoord);
	vec4 c1 = texture(texture1, v_texcoord);
	fragColor = vec4((( 1.0 - pct ) * c1 + pct * c2).rgb, alpha);
}
