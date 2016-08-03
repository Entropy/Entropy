in vec4 position;
in vec2 texcoord;

out vec2 f_texcoord;
out vec4 f_texcoord1;
out vec4 f_texcoord2;
#ifdef BLUR9
    out vec4 f_texcoord3;
    out vec4 f_texcoord4;
#endif
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

float combined_weight(float w1, float w2){
    return w1 + w2;
}

float combined_offset(float w1, float w2, float offset1, float offset2, float w12){
    return (offset1 * w1 + offset2 * w2) / w12;
}

void main(){

#ifdef BLUR9
    float w87 = combined_weight(w8, w7);
    float w65 = combined_weight(w6, w5);
#endif
    float w43 = combined_weight(w4, w3);
    float w21 = combined_weight(w2, w1);

#ifdef PASS_V
    float offset = texel_size.y;
#else
    float offset = texel_size.x;
#endif

#ifdef BLUR9
    float offset_8 = 8 * offset;
    float offset_7 = 7 * offset;
    float offset_6 = 6 * offset;
    float offset_5 = 5 * offset;
#endif
    float offset_4 = 4 * offset;
    float offset_3 = 3 * offset;
    float offset_2 = 2 * offset;
    float offset_1 = 1 * offset;

#ifdef BLUR9
    float offset_87 = combined_offset(w8, w7, offset_8, offset_7, w87);
    float offset_65 = combined_offset(w6, w5, offset_6, offset_5, w65);
#endif
    float offset_43 = combined_offset(w4, w3, offset_4, offset_3, w43);
    float offset_21 = combined_offset(w2, w1, offset_2, offset_1, w21);

#ifdef PASS_V
    f_texcoord1 = vec4(texcoord.x, texcoord.y - offset_21,
                       texcoord.x, texcoord.y + offset_21
                      );
    f_texcoord2 = vec4(texcoord.x, texcoord.y - offset_43,
                       texcoord.x, texcoord.y + offset_43
                      );
#ifdef BLUR9
    f_texcoord3 = vec4(texcoord.x, texcoord.y - offset_65,
                       texcoord.x, texcoord.y + offset_65
                      );
    f_texcoord4 = vec4(texcoord.x, texcoord.y - offset_87,
                       texcoord.x, texcoord.y + offset_87
                      );
#endif
#else
    f_texcoord1 = vec4(texcoord.x - offset_21, texcoord.y,
                       texcoord.x + offset_21, texcoord.y
                      );
    f_texcoord2 = vec4(texcoord.x - offset_43, texcoord.y,
                       texcoord.x + offset_43, texcoord.y
                      );
#ifdef BLUR9
    f_texcoord3 = vec4(texcoord.x - offset_65, texcoord.y,
                       texcoord.x + offset_65, texcoord.y
                      );
    f_texcoord4 = vec4(texcoord.x - offset_87, texcoord.y,
                       texcoord.x + offset_87, texcoord.y
                      );
#endif
#endif
    f_texcoord = texcoord;
    gl_Position = position;
}
