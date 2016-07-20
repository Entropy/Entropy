// Filename: prefilterEnvMap.frag
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Tue Nov 10 15:51:58 2015 (-0500)
// Last-Updated: Wed Nov 11 18:56:01 2015 (-0500)

#include "utils.glsl"
#include "pbr_frag.glsl"

// Hammersley implementation from here:
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
float radicalInverse_VdC( uint bits )
{
  bits = (bits << 16u) | (bits >> 16u);
  bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xaaaaaaaau) >> 1u);
  bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xccccccccu) >> 2u);
  bits = ((bits & 0x0f0f0f0fu) << 4u) | ((bits & 0xf0f0f0f0u) >> 4u);
  bits = ((bits & 0x00ff00ffu) << 8u) | ((bits & 0xff00ff00u) >> 8u);

  return float(bits) * 2.3283064365386963e-10; // / 0x100000000 
}

vec2 Hammersley( uint i, uint N )
{
    // why the fract?
    return vec2( float(i) / float(N), radicalInverse_VdC(i) );
}

// Importance Sampling (page 4 in course notes)
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 ImportanceSampleGGX( vec2 Xi, float roughness, vec3 N )
{
  float a = roughness * roughness;
  float Phi = 2 * PI * Xi.x;
  float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
  float SinTheta = sqrt( 1 - CosTheta * CosTheta );

  vec3 H;
  H.x = SinTheta * cos( Phi );
  H.y = SinTheta * sin( Phi );
  H.z = CosTheta;

  vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);

  vec3 TangentX = normalize( cross( UpVector, N ) );
  vec3 TangentY = cross( N, TangentX );

  // Tangent to world space
  return TangentX * H.x + TangentY * H.y + N * H.z;
}

// Integrate the BRDF into lookup texture (page 7 in course notes)
// http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec2 IntegrateBRDF( float Roughness, float NoV )
{
  vec3 V;
  V.x = sqrt( 1.0f - NoV * NoV ); // sin
  V.y = 0.0f;
  V.z = NoV; // cos

  float A = 0.0f;
  float B = 0.0f;
  
  const uint NumSamples = 1024u;
  for( uint i = 0u; i < NumSamples; i++ )
  {
    vec2 Xi = Hammersley( i, NumSamples );
    vec3 H = ImportanceSampleGGX( Xi, Roughness, vec3( 0.0, 0.0, 1.0 ) );
    vec3 L = 2.0 * dot( V, H ) * H - V;

    float NoL = saturate( L.z );
    float NoH = saturate( H.z );
    float VoH = saturate( dot( V, H ) );

    if( NoL > 0 )
    {
      float G = G_Smith( Roughness, NoV, NoL );
      float G_Vis = G * VoH / (NoH * NoV);
      float Fc = pow( 1.0 - VoH, 5.0 );
      A += (1.0 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }

  return vec2( A, B ) / float(NumSamples);
}

in vec2 vTexCoord0;

out vec4 oColor;

void main( void )
{
    float NoV = vTexCoord0.x;
    float roughness = 1.0f - vTexCoord0.y;

    vec2 BRDF = IntegrateBRDF( roughness, NoV ); 

    oColor = vec4( BRDF, 0.0f, 1.0f );
}
