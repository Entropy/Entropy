#version 150

uniform vec4 globalColor;

uniform float densityMin;
uniform float densityMax;

in vec4 vColor;

out vec4 fFragColor;

float map(float value, float inMin, float inMax, float outMin, float outMax)
{
    return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

void main (void)
{
    fFragColor = vec4(globalColor.rgb, map(vColor.a, densityMin, densityMax, 0.0, 1.0));
}
