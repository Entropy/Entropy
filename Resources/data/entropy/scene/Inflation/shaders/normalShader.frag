#version 450

uniform float isolevel;
const float resolution = 64.f;

#define SHADE_NORMALS 1
#if SHADE_NORMALS
in vec3 normal;
#endif
in vec4 rgba;

out vec4 vFragColor;

void main(void)
{
#if SHADE_NORMALS
    vFragColor = vec4(normal, 1.0);
#else
    //vFragColor = vec4(vec3(1.0), isolevel);
    vFragColor = rgba;
#endif
}
