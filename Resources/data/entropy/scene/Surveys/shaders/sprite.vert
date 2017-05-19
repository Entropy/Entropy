#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform float uMaxMass;
uniform float uMaxSize;
uniform mat4 uTransform;

uniform vec2 uClipRange;

uniform float uPointSize;
uniform float uAttenuation;

uniform float uCutRadius;
uniform float uMinRadius;
uniform float uMaxRadius;
uniform float uMinLatitude;
uniform float uMaxLatitude;
uniform float uMinLongitude;
uniform float uMaxLongitude;

in vec4 position;
in float mass;
in float starFormationRate;

flat out int vID;
flat out int vCell;

out float vAlpha;

void main()
{
	// Convert position from spherical to Cartesian coordinates.
	vec4 vertex = vec4(position.z * cos(position.y) * cos(position.x),
					   position.z * cos(position.y) * sin(position.x),
					   position.z * sin(position.y),
					   1.0);

	vec4 eyePos = modelViewMatrix * uTransform * vertex;
	gl_Position = projectionMatrix * eyePos;

	float eyeDist = length(eyePos.xyz);
	float attenuation = uAttenuation / eyeDist;
	//float attenuation = 1.0;

	float size = uPointSize * mass * attenuation;
	gl_PointSize = size;

	// Enable fragment if we're within range.
	if (//uMaxMass < mass ||
		//uMaxSize < size ||
		uCutRadius > position.z ||
		uMinLongitude > position.x || position.x > uMaxLongitude ||
		uMinLatitude > position.y || position.y > uMaxLatitude)
	{
		gl_Position.w = 0;  // Discards the vertex.
	}

	// Calculate alpha based on radius.
	if (position.z >= uMaxRadius)
	{
		gl_Position.w = 0;
	}
	else if (position.z <= uMinRadius)
	{
		vAlpha = 1.0;
	}
	else
	{
		// Map radius from 0.0 to 1.0.
		vAlpha = 1.0 - (position.z - uMinRadius) / (uMaxRadius - uMinRadius);
	}

	// Fade as we near the far clipping plane.
	if (eyeDist > uClipRange.x)
	{
		// Map distance from 0.0 to 1.0.
		vAlpha *= (1.0 - (eyeDist - uClipRange.x) / (uClipRange.y - uClipRange.x));
	}

	vAlpha = clamp(vAlpha, 0.0, 1.0);

	// Used for flipping texture.
	vID = gl_VertexID;

	// Determine the sprite cell based on type.
	if (starFormationRate < 0.0)
	{
		vCell = 0;
	}
	else if (starFormationRate == 0.0)
	{
		// Elliptical, boxes 1-7.
		vCell = gl_VertexID % 7 + 1;
	}
	else if (starFormationRate > 0.0)
	{
		// Spiral, boxes 8-15.
		vCell = gl_VertexID % 8 + 8;
	}
}
