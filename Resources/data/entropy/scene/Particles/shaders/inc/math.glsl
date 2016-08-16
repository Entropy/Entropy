// Filename: math.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov  7 15:56:55 2015 (-0500)
// Last-Updated: Thu Nov 12 09:36:53 2015 (-0500)

#ifndef RTK_MATH
#define RTK_MATH

#define PI      3.14159265359
#define INV_PI  0.31830988618
#define EPSILON 0.00000001

// ========================================================
//   Utility Functions
// ========================================================

// Value clamping to 0 - 1 range
float saturate( float value )
{
    return clamp( value, 0.0, 1.0 );
}

// Value clamping to 0 - 1 range
vec3 saturate( in vec3 value )
{
    return clamp( value, vec3( 0.0 ), vec3( 1.0 ) );
}

// https://www.shadertoy.com/view/4ssXRX
//note: uniformly distributed, normalized random
float nrand( vec2 n )
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

float random( vec2 n, float seed )
{
	float t = fract( seed );
	float nrnd0 = nrand( n + 0.07*t );
	float nrnd1 = nrand( n + 0.11*t );
	float nrnd2 = nrand( n + 0.13*t );
	float nrnd3 = nrand( n + 0.17*t );
	return (nrnd0+nrnd1+nrnd2+nrnd3) / 4.0;
}

// Ref: http://the-witness.net/news/2012/02/seamless-cube-map-filtering/
vec3 fixCubeLookup( vec3 _v, float _lod, float _topLevelCubeSize )
{
	float ax = abs(_v.x);
	float ay = abs(_v.y);
	float az = abs(_v.z);
	float vmax = max(max(ax, ay), az);
	float scale = 1.0 - exp2(_lod) / _topLevelCubeSize;
	if (ax != vmax) { _v.x *= scale; }
	if (ay != vmax) { _v.y *= scale; }
	if (az != vmax) { _v.z *= scale; }
	return _v;
}

#endif // RTK_MATH
