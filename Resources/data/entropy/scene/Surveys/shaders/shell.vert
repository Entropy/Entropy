#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform mat4 uTransform;

uniform float uPointSize;
uniform float uAttenuation;

uniform float uDensity;

uniform float uCutRadius;
uniform float uMinRadius;
uniform float uMaxRadius;
uniform float uMinLatitude;
uniform float uMaxLatitude;
uniform float uMinLongitude;
uniform float uMaxLongitude;

in vec4 position;

flat out int vEnabled;

void main()
{
	// Convert position from spherical to Cartesian coordinates.
	vec4 vertex = vec4(position.z * cos(position.y) * cos(position.x),
					   position.z * cos(position.y) * sin(position.x),
					   position.z * sin(position.y),
					   1.0);

	vec4 eyeCoord = modelViewMatrix * uTransform * vertex;
	gl_Position = projectionMatrix * eyeCoord;

	float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
	float attenuation = uAttenuation / dist;
	//float attenuation = 1.0;

	float size = uPointSize * attenuation;
	gl_PointSize = size;

	// Enable fragment if we're within range and density.
	if (gl_VertexID % int(uDensity) == 0 &&
		uCutRadius <= position.z &&
		uMinLongitude <= position.x && position.x <= uMaxLongitude &&
		uMinLatitude <= position.y && position.y <= uMaxLatitude)
	{
		vEnabled = 1;
	}
	else
	{
		vEnabled = 0;
	}
}
