#version 150

uniform sampler2D tex;

in float vType;
in vec4 vColor;

out vec4 fFragColor;

void main (void)
{
    if (vType == 0) {
        discard;
    }
    else {
        fFragColor = texture(tex, gl_PointCoord) * vColor;
    }
}
