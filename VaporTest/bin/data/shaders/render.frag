#version 150

uniform vec4 globalColor;

uniform float uDensityMin;
uniform float uDensityMax;

in float vDensity;
in float vDebug;

out vec4 fragColor;

float map(float value, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main (void)
{
//    if (vDebug > 0.0) {
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
//    }
//    else {
//        fragColor = vec4(globalColor.rgb, map(vDensity, uDensityMin, uDensityMax, 0.0, 1.0));
//    }
}
