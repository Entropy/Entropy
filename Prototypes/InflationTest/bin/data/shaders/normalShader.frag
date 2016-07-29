#version 450

uniform float isolevel;

in vec3 normal;
in vec4 rgba;

out vec4 vFragColor;

void main(void)
{
    //vFragColor = vec4(normal, 1.0);
    //vFragColor = vec4(vec3(1.0), isolevel);
    vFragColor = rgba;
}
