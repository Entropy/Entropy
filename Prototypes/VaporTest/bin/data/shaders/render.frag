#version 330
#define DISCARD_LOW_DENSITY 1

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
#if DISCARD_LOW_DENSITY
	if(vDensity<0.001){
		discard;
	}else{
#endif
		fragColor = vec4(globalColor.rgb, map(vDensity, uDensityMin, uDensityMax, 0.0, 1.0));
#if DISCARD_LOW_DENSITY
	}
#endif
}
