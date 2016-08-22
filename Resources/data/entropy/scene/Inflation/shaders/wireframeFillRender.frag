#version 330

in vec4 f_color;
in float f_distanceToCamera;

uniform float fogMaxDistance;
uniform float fogMinDistance;
uniform float fogPower;

out vec4 fragColor;

float fog(float dist, float minDist, float maxDist, float power) {
	dist = pow(dist, power);
	minDist = pow(minDist, power);
	maxDist = pow(maxDist, power);
	float invDistanceToCamera = 1 - clamp((dist - minDist) / maxDist, 0.f, 1.f);
	if (dist > minDist) {
		return invDistanceToCamera;
	}
	else {
		return 1;
	}
}

void main(void)
{
	fragColor = f_color;
	fragColor.a *= fog(f_distanceToCamera, fogMinDistance, fogMaxDistance, fogPower);
}
