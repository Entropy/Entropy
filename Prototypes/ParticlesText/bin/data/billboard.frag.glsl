#version 330

uniform sampler2D tex0;
uniform vec4 globalColor;
uniform float accumValue;
uniform float pctColor;
in vec2 v_texcoord;
flat in int v_valid;
in float v_depth;
out vec4 fragColor;

void main(void)
{
	if(v_valid==0){
		discard;
		return;
	}

	vec4 t = texture(tex0, v_texcoord);
	if(t.a==0){
		discard;
	}else{
		fragColor = vec4(t.rgb * pctColor, accumValue * t.a);
	}
}
