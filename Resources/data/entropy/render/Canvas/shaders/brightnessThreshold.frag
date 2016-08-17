#version 330

uniform sampler2D tex0;
uniform float bright_threshold;
out vec4 fragColor;

in vec2 f_texcoord;

float lumf(vec3 _rgb)
{
    return dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
}

float bright(vec3 rgb){
	return max(max(rgb.r, rgb.g), rgb.b);
}

void main(){
    vec4 color = texture(tex0, f_texcoord);
    if(bright(color.rgb)>bright_threshold){
        fragColor = vec4(clamp(color.rgb, vec3(0), vec3(100)), 1);
    }else{
        discard;
    }
}
