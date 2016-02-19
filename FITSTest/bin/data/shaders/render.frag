#version 150

uniform vec4 globalColor;

uniform sampler2D texx;

out vec4 fFragColor;

void main(void)
{
    fFragColor = texture(texx, gl_PointCoord) * globalColor;
}
