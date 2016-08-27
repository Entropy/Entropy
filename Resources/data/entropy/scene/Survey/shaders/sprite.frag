#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

in float vEnabled;

out vec4 fragColor;

void main(void)
{
    if (vEnabled < 0.5) fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	
	else
	fragColor = texture(uTex0, gl_PointCoord) * globalColor;
}
