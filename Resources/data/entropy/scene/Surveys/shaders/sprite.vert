#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform float uPointSize;
uniform float uMinRadius;
uniform float uMaxRadius;
uniform float uMinLatitude;
uniform float uMaxLatitude;
uniform float uMinLongitude;
uniform float uMaxLongitude;

in vec4 position;
in float mass;
in float starFormationRate;

flat out int vEnabled;
flat out int vIdentifier;
flat out float vStarFormationRate;

void main()
{
	// Convert position from spherical to Cartesian coordinates.
	vec4 vertex = vec4(position.z * cos(position.y) * cos(position.x),
						position.z * cos(position.y) * sin(position.x),
						position.z * sin(position.y),
						1.0);
	
	vec4 eyeCoord = modelViewMatrix * vertex;
	gl_Position = projectionMatrix * eyeCoord;

	float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
	float attenuation = 600.0 / dist;

	gl_PointSize = uPointSize * mass * attenuation;
		
	if (uMinRadius <= position.z && position.z <= uMaxRadius &&
		uMinLongitude <= position.x && position.x <= uMaxLongitude &&
		uMinLatitude <= position.y && position.y <= uMaxLatitude)
	{
		vEnabled = 1;
	}
	else
	{
		vEnabled = 0;
	}

	vIdentifier = gl_VertexID;
	vStarFormationRate = starFormationRate;
}
