#version 150

uniform vec4 globalColor;

uniform float densityMin;
uniform float densityMax;

in float vDensity;
in float vDebug;

out vec4 fFragColor;

float map(float value, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main (void)
{
    if (vDebug > 0.0) {
        fFragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else {
        fFragColor = vec4(globalColor.rgb, map(vDensity, densityMin, densityMax, 0.0, 1.0));
    }
}
