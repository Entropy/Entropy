// Filename: pbr.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov  7 15:57:32 2015 (-0500)
// Last-Updated: Sat Nov 14 15:18:09 2015 (-0500)

#ifndef RTK_PBR
#define RTK_PBR

#pragma include <toneMapping.glsl>

const float RCP_4PI = 1.0 / ( 4.0 * PI );

uniform vec4  uBaseColor;
uniform float uMetallic;
uniform float uRoughness;
uniform vec4  uEmissiveColor;
uniform float uEmissiveIntensity;

uniform samplerCube uIrradianceMap;
uniform samplerCube uRadianceMap;


// ===================================================
//   Normal Distribution Function (NDF / N Term )
// ===================================================

// Disney's GGX/Trowbridge-Reitz (Epic's UE4 paper)
float N_GGX( float roughness, float NoH )
{
  float a = roughness * roughness;
  float a2 = a * a;
  float denom = PI * pow( ( ( NoH * NoH ) * ( a2 - 1.0f ) + 1.0f ), 2 );
  return a2 / denom;
}


// ===================================================
//   Geometric Shadowing (G Term)
// ===================================================

// Schlick approximation of GGX 
float GGX( float NdotV, float roughness )
{
  float k = ( roughness * roughness ) / 2;
  return NdotV / (NdotV * (1.0f - k) + k);
}

float G_Smith(float roughness, float NdotV, float NdotL )
{
  return GGX( NdotL, roughness ) * GGX( NdotV, roughness );
}


// ===================================================
//   Fresnel Term 
// ===================================================

// Schlick Fresnel Approximation
vec3 F_Schlick( vec3 f0, float VdotH )
{
  // Epic's UE4 gaussian approximation is used here
  return f0 + (1.0 - f0) * exp2( ( -5.55473 * VdotH - 6.98316 ) * VdotH );
}

// ===================================================
//   Image Based Lighting 
// ===================================================

// UE4 Specular Image Based Lighting
vec3 ApproximateSpecularIBL( samplerCube RadianceSampler, sampler2D DFGSampler, vec3 SpecularColor, float Roughness, float NoV, vec3 R )
{
	int numMips = 7;
	float mipLevel = numMips - 1 + log2( Roughness );

	vec3 PrefilteredColor = textureLod( RadianceSampler, R, mipLevel ).rgb;
	PrefilteredColor = gammaToLinear( PrefilteredColor, uGamma );
	
	vec2 EnvBRDF = texture( DFGSampler, vec2( NoV, Roughness ) ).xy;

	return PrefilteredColor * ( SpecularColor * EnvBRDF.x + vec3( EnvBRDF.y ) );
}

// UE4's approximation for mobile devices 
// https://www.unrealengine.com/blog/physically-based-shading-on-mobile
vec3 EnvBRDFApprox( vec3 SpecularColor, float Roughness, float NoV )
{
	const vec4 c0 = vec4( -1, -0.0275, -0.572, 0.022 );
	const vec4 c1 = vec4( 1, 0.0425, 1.04, -0.04 );
	vec4 r = Roughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
	vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}



// ===================================================
//   Lighting Attenuation
// ===================================================

// Attenuation method described in Frostbite's paper (page 32)
float smoothDistanceAtt( float squaredDistance, float invSqrAttRadius )
{
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0f - factor * factor );
	return smoothFactor * smoothFactor;
}

// Attenuation method described in Frostbite's paper (page 32)
float getDistanceAtt( vec3 lightPosition, vec3 vertexPosition, float lightRadius )
{
	float distToLight = length( vertexPosition - lightPosition );
	float att = 1.0 / ( max( distToLight * distToLight, 0.01* 0.01 ) );
	att *= smoothDistanceAtt( distToLight * distToLight, ( 1.0 / ( lightRadius * lightRadius ) ) );
	return att;
}

// Attenuation method described in Epic's UE4 paper
float CalcPointLightAttenuation( vec3 lightPosition, vec3 vertexPosition, float lightRadius )
{
	float distToLight = length( vertexPosition - lightPosition );
	float falloff = pow( saturate( 1.0f - pow( distToLight / lightRadius, 4 ) ), 2 ) / ( distToLight * distToLight + 1.0f );
	return falloff;
}

void CalcPointLight( const in PointLight _light, 
					 const in vec3 _vertex,
					 const in vec3 _normal, 
					 const in vec3 _viewDir, 
					 const in float _NoV,
					 const in float _roughness,
					 const in vec3 _specularColor,
					 out vec3 _diffuseContrib, out vec3 _specularContrib )
{
	vec3 L = normalize( _light.position.xyz - _vertex.xyz );    // light direction
	vec3 H = normalize( _viewDir + L );                              // half vector

	float NoL = saturate( dot( _normal, L ) ) + EPSILON;
	float VoH = saturate( dot( _viewDir, H ) ) + EPSILON;
	float NoH = saturate( dot( _normal, H ) ) + EPSILON;

	// brdf terms
	float distribution = N_GGX( _roughness, NoH );
	vec3 fresnel       = F_Schlick( _specularColor, VoH );
	float geom         = G_Smith( _roughness, _NoV, NoL );

	// page 29-30 in Frostbite PDF
	// convert lumens to luminous intensity (cd)
	// there's 4PI steridians in a sphere
	// distToLight * distToLight is absent as attenuation adds that below 
	float luminousIntensity = _light.intensity * RCP_4PI;

	// I * NoL * BRDF * attenuation
	// with lambertian diffuse, albedo color IS your BRDF
	vec3 diffuse = luminousIntensity * NoL * _light.color.rgb;
	vec3 specular = luminousIntensity * NoL * ( distribution * fresnel * geom ); // *_specularColor;

	// apply attenuation falloff
	float attenuation = CalcPointLightAttenuation( _light.position.xyz, _vertex.xyz, _light.radius );

	diffuse *= attenuation;
	specular *= attenuation;

	_diffuseContrib = diffuse;
	_specularContrib = specular;
}

void CalcDirectionalLight( const in DirectionalLight _light, 
						   const in vec3 _vertex,
						   const in vec3 _normal, 
						   const in vec3 _viewDir,
						   const in float _NoV,
						   const in float _roughness,
						   const in vec3 _specularColor,
						   out vec3 _diffuseContrib, out vec3 _specularContrib )
{
	vec3 L = _light.direction.xyz;
	vec3 H = normalize( _viewDir + L );                              // half vector

	float NoL = saturate( dot( _normal, L ) ) + EPSILON;
	float VoH = saturate( dot( _viewDir, H ) ) + EPSILON;
	float NoH = saturate( dot( _normal, H ) ) + EPSILON;

	// brdf terms
	float distribution = N_GGX( _roughness, NoH );
	vec3 fresnel       = F_Schlick( _specularColor, VoH );
	float geom         = G_Smith( _roughness, _NoV, NoL );

	// page 29-30 in Frostbite PDF
	// convert lumens to luminous intensity (cd)
	// there's 4PI steridians in a sphere
	// distToLight * distToLight is absent as attenuation adds that below 
	float luminousIntensity = _light.intensity * RCP_4PI;

	// I * NoL * BRDF * attenuation
	// with lambertian diffuse, albedo color IS your BRDF
	vec3 diffuse = luminousIntensity * NoL * _light.color.rgb;
	vec3 specular = luminousIntensity * NoL * ( distribution * fresnel * geom ); // *_specularColor;

	_diffuseContrib = diffuse;
	_specularContrib = specular;
}

vec3 CalcIBLDiffuse( samplerCube _irradianceMap, vec3 _normal, float intensity )
{
	vec3 diffuseIBL = texture( _irradianceMap, _normal ).rgb;
	return diffuseIBL * INV_PI * ambientIntensity;
}

vec3 CalcIBLSpecular( samplerCube _radianceMap, int _numMips, vec3 _reflectNormal, float _NoV, float _roughness, float _intensity )
{
	float mipLevel = _numMips - 1 + log2( _roughness );
	vec3 PrefilteredColor = textureLod( _radianceMap, _reflectNormal, mipLevel ).rgb;

	vec3 specularIBL = EnvBRDFApprox( PrefilteredColor, _roughness, _NoV );
	return specularIBL * INV_PI * _intensity;
}

#endif // RTK_PBR
