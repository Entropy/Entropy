// Filename: color_tools.glsl
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Mar 26 10:57:15 2016 (-0400)
// Last-Updated: Sat Mar 26 10:57:42 2016 (-0400)

uniform float       uGamma;
uniform float       uExposure;

// ========================================================
//   Gamma / Linear Space Conversion 
// ========================================================

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
// ========================================================

// Uncharted 2 Tone Mapping http://filmicgames.com/archives/75
vec3 tonemapUncharted2( vec3 color )
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

// Reinhard Tone Mapping (http://www.cs.utah.edu/~reinhard/cdrom/tonemap.pdf)
vec3 tonemapReinhard( vec3 color )
{
    return color / ( vec3( 1.0 ) + color );
}

