#version 150

uniform vec4 globalColor;

uniform sampler2D uTex0;

flat in int vEnabled;
flat in int vIdentifier;
flat in float vStarFormationRate;

out vec4 fragColor;

void main(void)
{
	if (vEnabled == 0) discard;

	vec2 texCoord = gl_PointCoord;

	if (vIdentifier % 3 == 0)
	{
		texCoord.x = 1.0 - texCoord.x;
	}
	if (vIdentifier % 5 == 0)
	{
		texCoord.y = 1.0 - texCoord.y;
	}

	texCoord *= 0.25;

	if (vStarFormationRate == 0.0)
	{
		// Elliptical, boxes 1-7.
		int idx = vIdentifier % 7 + 1;
		int col = idx % 4;
		texCoord.x += col * 0.25;
		if (idx > 4) texCoord.y += 0.25;
	}
	else if (vStarFormationRate > 0.0)
	{
		// Spiral, boxes 8-15.
		texCoord.y += 0.5;

		int idx = vIdentifier % 8;
		int col = idx % 4;
		texCoord.x += col * 0.25;
		if (idx > 4) texCoord.y += 0.25;
	}
	//else // (vStarFormationRate < 0.0)
	//{
		// Star, box 1.
	//}

	fragColor = texture(uTex0, texCoord) * globalColor;
}
