#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

flat in int vEnabled;
flat in int vID;
flat in int vCell;

in float vAlpha;

out vec4 fragColor;

void main(void)
{
	if (vEnabled == 0) discard;

	vec2 texCoord = gl_PointCoord;

	if (vCell == 0)
	{
		// Star, cell 0.
		texCoord *= 0.25;
	}
	else
	{
		// Galaxy, cells 1-15.
		int col = vCell % 4;
		int row = vCell / 4;

		// Flip the texture for fun.
		if (vID % 3 == 0)
		{
			texCoord.x = (1.0 - texCoord.x + col) * 0.25;
		}
		else
		{
			texCoord.x = (texCoord.x + col) * 0.25;
		}
		if (vID % 5 == 0)
		{
			texCoord.y = (1.0 - texCoord.y + row) * 0.25;
		}
		else
		{
			texCoord.y = (texCoord.y + row) * 0.25;
		}
	}

	fragColor = vec4((texture(uTex0, texCoord) * globalColor).rgb, vAlpha);
}
