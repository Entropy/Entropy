#version 330
#define DISCARD_LOW_DENSITY 1

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

in float density;
in vec4 position_size;
uniform float uDensityMin;
uniform float uDensityMax;


//uniform samplerBuffer uTransform;

out float vDensity;
const float maxDistance = 1024.;
const float sqMaxDistance = maxDistance * maxDistance;
const float pointSizeFactor = 50000000.;
const vec3 scale = vec3(1.000001);

void main()
{
#if DISCARD_LOW_DENSITY
	float threshold = uDensityMin + (uDensityMax-uDensityMin) * 0.002;
	if(density<threshold){
		gl_Position = vec4(0.0);
		vDensity = 0.0;
	}else{
#endif
		vec4 transformPosition = vec4(position_size.xyz, 1.0);
		vec4 eyePosition = modelViewMatrix * transformPosition;

		vDensity = density;

		gl_Position = projectionMatrix * eyePosition;

		vec4 projVoxel = projectionMatrix * vec4(position_size.w * pointSizeFactor, 0.0, eyePosition.z, eyePosition.w);
		gl_PointSize = maxDistance * projVoxel.x / projVoxel.w;
#if DISCARD_LOW_DENSITY
	}
#endif
}
