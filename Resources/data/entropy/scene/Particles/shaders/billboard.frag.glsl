#version 330

uniform vec4 globalColor;
uniform float accumValue;
uniform float pctColor;

#define HAS_TEXTURE 1

#if HAS_TEXTURE
uniform sampler2D tex0;
in vec2 v_texcoord;
#endif

flat in int v_valid;
in float v_depth;
out vec4 fragColor;

void main(void)
{
	if(v_valid==0){
		discard;
		return;
	}

#if HAS_TEXTURE
	vec4 t = texture(tex0, v_texcoord);
	if(t.a==0){
		discard;
	}else{
		fragColor = vec4(globalColor.rgb * t.rgb * pctColor, accumValue * globalColor.a * t.a);
	}
#else
	fragColor = vec4(globalColor.rgb * pctColor, accumValue * globalColor.a);
#endif
}
