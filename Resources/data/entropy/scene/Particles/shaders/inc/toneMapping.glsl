// Filename: toneMapping.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Mar 26 10:57:15 2016 (-0400)
// Last-Updated: Sat Mar 26 10:57:42 2016 (-0400)

#ifndef RTK_TONE_MAPPING
#define RTK_TONE_MAPPING

#pragma include <math.glsl>

uniform float uGamma;
uniform float uExposure;
uniform int uTonemapType;

// ========================================================
//   Color Operators
// ========================================================

float colorToLuminance( vec3 color )
{
	//const vec3 A = vec3( 0.2126729, 0.7151522, 0.0721750 );
	return dot( vec3( 0.2126729, 0.7151522, 0.0721750 ), color );
}

vec3 gammaToLinear( vec3 color, float gamma )
{
	return pow( color, vec3( gamma ) );
}

vec3 linearToGamma( vec3 color, float gamma )
{
	return pow( color, vec3( 1.0 / gamma ) );
}

// ========================================================
//   HDR Tonemapping Operators 
//   note: input should already be corrected for exposure
//         output will require gamma correction
// ========================================================

// Uncharted 2 Tone Mapping http://filmicgames.com/archives/75
vec3 tonemapUncharted2( vec3 color )
{
	const float UA = 0.15;
	const float UB = 0.50;
	const float UC = 0.10;
	const float UD = 0.20;
	const float UE = 0.02;
	const float UF = 0.30;

	return ( ( color * ( UA * color + UC * UB ) + UD * UE ) / ( color * ( UA * color + UB ) + UD * UF ) ) - UE / UF;
}

// Reinhard Tone Mapping (http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf)
vec3 tonemapReinhard( vec3 color )
{
	return color / ( vec3( 1.0 ) + color );
}

vec3 tonemapReinhardLum( vec3 color, float whiteSq )
{
	float lumVal = colorToLuminance( color );
    float lumScale = ( lumVal * ( 1.0 + lumVal / whiteSq ) ) / ( 1.0 + lumVal );

	return ( color * lumScale / lumVal );
}

vec3 tonemapFilmic( vec3 color )
{
	color = max( vec3( 0.0 ), color - 0.004 );
	color = ( color * ( 6.2 * color + 0.5 ) ) / ( color * ( 6.2 * color + 1.7 ) + 0.06 );
	return color;
}

// ACES Tone Mapping 
vec3 tonemapACES( vec3 color )
{
	const float AA = 2.51f;
	const float AB = 0.03f;
	const float AC = 2.43f;
	const float AD = 0.59f;
	const float AE = 0.14f;

	return saturate( ( color * ( AA * color + AB) ) / ( color * ( AC * color + AD ) + AE) );
}

// ========================================================
//   Color Correction Functions
// ========================================================

vec3 colorCorrect( in vec3 color, in float exposure, in int tonemapType, in float gamma )
{
	// Exposure.
	vec3 result = color * exposure;

	// Tone mapping.
	if ( tonemapType == 1 ) // Uncharted2
	{
		result = tonemapUncharted2( result );

		// Correct for white input level.
		const float whiteInputLevel = 2.0;
		vec3 whiteScale = 1.0 / tonemapUncharted2( vec3( whiteInputLevel ) );
		result = result * whiteScale;
	}
	else if ( tonemapType == 2 ) // Reinhard
	{
		result = tonemapReinhard( result );
	}
	else if ( tonemapType == 3 ) // Reinhard Luminance
	{
		const float whiteSq = 1.0;
		result = tonemapReinhardLum( result, whiteSq );
	}
	else if ( tonemapType == 4 ) // Filmic
	{
		result = tonemapFilmic( result );
	}
	else if ( tonemapType == 5 ) // ACES
	{
		result = tonemapACES( result );
	}

	// Gamma correction.
	return linearToGamma( result, gamma );
}

vec4 colorCorrect( in vec4 color, in float exposure, in int tonemapType, in float gamma )
{
	return vec4( colorCorrect( color.rgb, exposure, tonemapType, gamma ), color.a );
}

#endif // RTK_TONE_MAPPING
