#version 330

uniform mat4 modelViewProjectionMatrix;
uniform sampler2DRect particles0;
uniform sampler2DRect particles1;

in vec4 position;
in vec2 texcoord;
in vec4 color;

out float ageVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;

void main()
{
    texCoordVarying = texcoord;
    vec4 pos = texture(particles0, texCoordVarying);
    ageVarying = pos.w;
    colorVarying = color;
    gl_PointSize = ageVarying * color.a * 10.0;
    gl_Position = modelViewProjectionMatrix * vec4(pos.xyz, 1.0);
}
