


vec4 blur9(sampler2D _sampler
         , vec2 _uv0
         , vec4 _uv1
         , vec4 _uv2
         , vec4 _uv3
         , vec4 _uv4
         , float _weight0
         , float _weight1
         , float _weight2
         , float _weight3
         , float _weight4
         , float level
         )
{
    vec4 blur;
    blur  = texture(_sampler, _uv0)   *_weight0;
    blur += texture(_sampler, _uv1.xy)*_weight1;
    blur += texture(_sampler, _uv1.zw)*_weight1;
    blur += texture(_sampler, _uv2.xy)*_weight2;
    blur += texture(_sampler, _uv2.zw)*_weight2;
    blur += texture(_sampler, _uv3.xy)*_weight3;
    blur += texture(_sampler, _uv3.zw)*_weight3;
    blur += texture(_sampler, _uv4.xy)*_weight4;
    blur += texture(_sampler, _uv4.zw)*_weight4;
    return blur;
}

vec4 blur5(sampler2D _sampler
         , vec2 _uv0
         , vec4 _uv1
         , vec4 _uv2
         , float _weight0
         , float _weight1
         , float _weight2
         , float level
         )
{
    vec4 blur;
    blur  = texture(_sampler, _uv0)   *_weight0;
    blur += texture(_sampler, _uv1.xy)*_weight1;
    blur += texture(_sampler, _uv1.zw)*_weight1;
    blur += texture(_sampler, _uv2.xy)*_weight2;
    blur += texture(_sampler, _uv2.zw)*_weight2;
    return blur;
}


float combined_weight(float w1, float w2){
    return w1 + w2;
}

uniform sampler2D tex0;
uniform vec2 texel_size;
#ifdef BLUR9
    uniform float w8;
    uniform float w7;
    uniform float w6;
    uniform float w5;
#endif
uniform float w4;
uniform float w3;
uniform float w2;
uniform float w1;
uniform float w0;
in vec2 f_texcoord;
in vec4 f_texcoord1;
in vec4 f_texcoord2;
#ifdef BLUR9
    in vec4 f_texcoord3;
    in vec4 f_texcoord4;
#endif

out vec4 fragColor;

void main(){
#ifdef BLUR9
    float w87 = combined_weight(w8, w7);
    float w65 = combined_weight(w6, w5);
    float w43 = combined_weight(w4, w3);
    float w21 = combined_weight(w2, w1);
    fragColor =  blur9(tex0,
        f_texcoord,
        f_texcoord1,
        f_texcoord2,
        f_texcoord3,
        f_texcoord4,
        w0, w21, w43, w65, w87,
        0
    );
#else
    float w43 = combined_weight(w4, w3);
    float w21 = combined_weight(w2, w1);
    fragColor =  blur5(tex0,
        f_texcoord,
        f_texcoord1,
        f_texcoord2,
        w0, w21, w43,
        0
    );
#endif
}
