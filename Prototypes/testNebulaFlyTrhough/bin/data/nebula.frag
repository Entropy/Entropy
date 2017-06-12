#version 330

uniform sampler2D texture0;
uniform float colorramp_low;
uniform float colorramp_high;
uniform vec4 globalColor;

in vec2 v_texcoord;
out vec4 fragColor;

vec4 map(vec4 v, float inmin, float inmax, float outmin, float outmax){
	float inrange = (inmax - inmin);
	float outrange = (outmax - outmin);
	return (v - inmin) / inrange * outrange + outmin;
}

void main(void)
{
	vec4 c = texture(texture0, v_texcoord);
	c = map(c, colorramp_low, colorramp_high, 0., 1.);
	fragColor = c * globalColor;
}
