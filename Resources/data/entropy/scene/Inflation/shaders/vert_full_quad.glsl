#version 330

in vec4 position;
in vec2 texcoord;

out vec2 f_texcoord;

void main(){
    f_texcoord = texcoord;
    gl_Position = position;
}
