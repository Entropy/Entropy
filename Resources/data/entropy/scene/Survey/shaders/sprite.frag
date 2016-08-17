#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(uTex0, gl_PointCoord) * globalColor;
}
