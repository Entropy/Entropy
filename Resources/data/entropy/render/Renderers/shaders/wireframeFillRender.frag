#version 330

in vec4 f_color;
in float f_distanceToCamera;
in float f_distanceToCenter;

uniform float fogMaxDistance;
uniform float fogMinDistance;
uniform float fogStartDistance;
uniform float fogPower;
uniform float fadeEdge0;
uniform float fadeEdge1;
uniform float fadePower;

out vec4 fragColor;
#define FOG_ENABLED 0
#define SPHERICAL_CLIP 0


float fog(float dist, float startDist, float minDist, float maxDist, float power) {
	return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
}

void main(void)
{
	fragColor = f_color;
#if FOG_ENABLED
	fragColor *= fog(f_distanceToCamera, fogStartDistance, fogMinDistance, fogMaxDistance, fogPower);
#endif
#if SPHERICAL_CLIP
	float sphere = 1 - pow(smoothstep(fadeEdge0, fadeEdge1, f_distanceToCenter), fadePower);
	fragColor.a *= sphere;
#endif
}
