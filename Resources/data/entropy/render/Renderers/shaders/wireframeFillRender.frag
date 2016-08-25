#version 330

in vec4 f_color;
in float f_distanceToCamera;

uniform float fogMaxDistance;
uniform float fogMinDistance;
uniform float fogStartDistance;
uniform float fogPower;

out vec4 fragColor;
#define FOG_ENABLED 0


float fog(float dist, float startDist, float minDist, float maxDist, float power) {
	return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
}

void main(void)
{
	fragColor = f_color;
#if FOG_ENABLED
	fragColor.a *= fog(f_distanceToCamera, fogStartDistance, fogMinDistance, fogMaxDistance, fogPower);
#endif
}
