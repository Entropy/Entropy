#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform float uPointSize;
uniform float uMinLatitude;
uniform float uMaxLatitude;
uniform float uMinLongitude;
uniform float uMaxLongitude;

in vec4 position;

out float vEnabled;

void main()
{
	if (uMinLongitude <= position.x && position.x <= uMaxLongitude &&
		uMinLatitude <= position.y && position.y <= uMaxLatitidue)
	{
		vEnabled = 1.0;
		
		// Convert position from spherical to Cartesian coordinates.
		vec4 vertex = vec4(position.z * cos(position.y) * cos(position.x),
						   position.z * cos(position.y) * sin(position.x),
						   position.z * sin(position.y),
						   1.0);
	
		vec4 eyeCoord = modelViewMatrix * vertex;
		gl_Position = projectionMatrix * eyeCoord;

		float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
		float attenuation = 600.0 / dist;

		gl_PointSize = uPointSize * position.w * attenuation;
	}
	else
	{
		vEnabled = 0.0;
	}
}
