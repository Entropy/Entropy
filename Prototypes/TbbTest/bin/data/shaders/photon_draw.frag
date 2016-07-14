#version 330

uniform sampler2DRect tex;

in vec4 colorVarying;
in float ageVarying;

out vec4 fragColor;

void main()
{
    fragColor = colorVarying * ageVarying * texture(tex, 400.0 * gl_PointCoord);//vec4(1.0, 1.0, 1.0, 0.1);
}
