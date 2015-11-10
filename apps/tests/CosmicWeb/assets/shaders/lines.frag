#version 150

out vec4          oColor;
in float          vDistance;

uniform float     uFog;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
            in float distance, in float strength ) // camera to point distance
{
float fogAmount = 1.0 - exp( -distance * strength );
vec3  fogColor  = vec3(0.0);
return mix( rgb, fogColor, fogAmount );
}

void main() {
  vec3 color    = vec3( 1.0 );
  color         = applyFog( color, vDistance, 0.09 );
  oColor        = vec4( pow( color, vec3(1.1) ), 0.25 );
}