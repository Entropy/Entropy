

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

float distanceToCamera(vec3 p){
	vec4 eyePosition =  modelViewMatrix * vec4(p,1.0f);
	return length(eyePosition);
}

const float a = 0.22; // Shoulder strength
const float b = 0.30; // Linear strength
const float c = 0.10; // Linear angle
const float d = 0.20; // Toe Strength
const float e = 0.01; // Toe numerator
const float f = 0.30; // Toe denominator
					  // e/f Toe angle
const float w = 2; // Linear white point value

/*uniform float exposureBias;
uniform float a; // Shoulder strength
uniform float b; // Linear strength
uniform float c; // Linear angle
uniform float d; // Toe Strength
uniform float e; // Toe numerator
uniform float f; // Toe denominator
					  // e/f Toe angle
uniform float w = 11.2; // Linear white point value*/

float fog2(float _x)
{
    return ((_x*(a*_x+c*b)+d*e)/(_x*(a*_x+b)+d*f))-e/f;
}