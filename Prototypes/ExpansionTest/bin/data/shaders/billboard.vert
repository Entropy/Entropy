#version 150

uniform mat4 orientationMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform float pointSize;

in vec4 position;
in vec3 normal;

out float vType;
out vec4 vColor;

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
	vec4 eyeCoord = modelViewMatrix * position;
	gl_Position = projectionMatrix * eyeCoord;
	
	float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
	float attenuation = 600.0 / dist;
	
//	gl_PointSize = normal.x * attenuation;
    gl_PointSize = pointSize * attenuation;

    vType = 1.0; //normal.y;

    float sat = clamp(mix(1.0, 0.0, attenuation), 0.0, 1.0);
    vColor = vec4(hsv2rgb(vec3(0.0, sat, 1.0)), 1.0);
}





