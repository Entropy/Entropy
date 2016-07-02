#version 330
#pragma include "defines.glsl"

uniform vec4 globalColor;

uniform float uDensityMin;
uniform float uDensityMax;

in float vDensity;

out vec4 fragColor;

float map(float value, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main (void)
{
#if DEBUG
	    fragColor = vec4(1.0);
#else
	    fragColor = vec4(globalColor.rgb, map(vDensity, uDensityMin, uDensityMax, 0.0, 1.0));
#endif
}
