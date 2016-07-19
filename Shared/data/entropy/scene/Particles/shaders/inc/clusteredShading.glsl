// Filename: clusteredShading.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov  7 15:56:55 2015 (-0500)
// Last-Updated: Thu Nov 12 09:36:53 2015 (-0500)

#ifndef RTK_CLUSTERED_SHADING
#define RTK_CLUSTERED_SHADING

const int MAX_POINT_LIGHTS = 1024;
const int MAX_DIRECTIONAL_LIGHTS = 4;

// Clustering slice setup.
const float NUM_SLICES_X = 20.0f;
const float NUM_SLICES_Y = 11.0f;
const float NUM_SLICES_Z = 16.0f;

struct PointLight
{
	vec4  position;
	vec3  color;
	float radius;
	float intensity;
};

struct DirectionalLight
{
	vec4  direction;
	vec3  color;
	float intensity;
};

layout (std140) uniform uLightBlock
{
	PointLight       pointLights[ MAX_POINT_LIGHTS ];
	DirectionalLight directionalLights[ MAX_DIRECTIONAL_LIGHTS ];
	int              directionalLightCount;
	float            ambientIntensity;
};

uniform usampler3D     uLightPointerTex;
uniform usamplerBuffer uLightIndexTex;

vec3 GetLightOffsetAndCount( const in vec2 _coord_xy, const in float _vertexZ, out int _lightIndexOffset, out int _pointLightCount )
{
	float linearDepth = ( -_vertexZ - viewData.nearClip ) / ( viewData.farClip - viewData.nearClip );
	int slice = int( max( linearDepth * NUM_SLICES_Z, 0.0f ) );

	//const float min_depth = log2( 0.1f );
	//const float max_depth = log2( 1.0f );
	//const float scale = 1.0f / ( max_depth - min_depth ) * ( NUM_SLICES_Z - 1.0f );
	//const float bias = 1.0f - min_depth * scale;

	ivec3 cluster_coord;
	cluster_coord.xy = ivec2( _coord_xy * viewData.rcpViewportSize* ivec2( NUM_SLICES_X, NUM_SLICES_Y ) );
	cluster_coord.z = slice;

	uvec4 light_data = texelFetch( uLightPointerTex, cluster_coord, 0 );
	_lightIndexOffset = int( light_data.x );
	_pointLightCount = int( light_data.y );

	return cluster_coord;
}

PointLight GetPointLight( int _lightIndex )
{
	uint index = texelFetch( uLightIndexTex, _lightIndex ).r; 
	return pointLights[ index ];
}

#endif // RTK_CLUSTERED_SHADING