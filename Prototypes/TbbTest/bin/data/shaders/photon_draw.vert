#version 330

uniform mat4 modelViewProjectionMatrix;
uniform sampler2DRect particles0;
uniform sampler2DRect particles1;
uniform float universeAge;

in vec4 position;
in vec2 texcoord;
in vec4 color;

out float ageVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;

vec3 hsv(float h,float s,float v) { return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v; }

void main()
{
    texCoordVarying = texcoord;
    vec4 pos = texture(particles0, texCoordVarying);
    ageVarying = pos.w;
    colorVarying = vec4(hsv(clamp(universeAge - 0.1 + color.r, 0.0, 1.0), 0.5, 1.0), color.a);
    gl_PointSize = ageVarying * color.a * 10.0;
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);
}
