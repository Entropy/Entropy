#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform float uPointSize;

in vec4 position;
in float mass;

void main()
{
    vec4 eyeCoord = modelViewMatrix * position;
    gl_Position = projectionMatrix * eyeCoord;

    float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
    float attenuation = 600.0 / dist;

    gl_PointSize = uPointSize * mass * attenuation;
}
