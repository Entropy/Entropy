#version 330
#pragma include "defines.glsl"

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

in vec3 position;
in float size;
in float density;
uniform float uDensityMin;
uniform float uDensityMax;


//uniform samplerBuffer uTransform;

out float vDensity;
const float maxDistance = 1024.;
const float sqMaxDistance = maxDistance * maxDistance;
const float pointSizeFactor = 50000000.;

void main()
{
	vec4 transformPosition = vec4(position, 1.0);
	vec4 eyePosition = modelViewMatrix * transformPosition;

	vDensity = density;

	gl_Position = projectionMatrix * eyePosition;

#if DEBUG
	vec4 projVoxel = projectionMatrix * vec4(size*0.0000000001 * pointSizeFactor, 0.0, eyePosition.z, eyePosition.w);
	gl_PointSize = maxDistance * projVoxel.x / projVoxel.w;
#else
	vec4 projVoxel = projectionMatrix * vec4(size * pointSizeFactor, 0.0, eyePosition.z, eyePosition.w);
	gl_PointSize = maxDistance * projVoxel.x / projVoxel.w;
#endif
}
