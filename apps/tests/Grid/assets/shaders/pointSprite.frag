#version 150


uniform sampler2D uTexture;
out vec4          oColor;
in float          vDistance;
in float          vBubbleFlag;
uniform float     uFog;

vec3 applyFog( in vec3  rgb,       // original color of the pixel
            in float distance, in float strength ) // camera to point distance
{
    float fogAmount = 1.0 - exp( -distance * strength );
    vec3  fogColor  = vec3(0.0);
    return mix( rgb, fogColor, fogAmount );
}

void main() {
    vec3 color    = texture( uTexture, gl_PointCoord.xy ).rgb;
    vec3 color2   = texture( uTexture, ( gl_PointCoord.xy * 2.0 - vec2(1.0) ) * 0.2 + vec2( 0.5 ) ).rgb;
    color         = mix( color, color * vec3( 1,0,0 ), vBubbleFlag );
    color         = applyFog( color, vDistance * vDistance, uFog );// mix( uFog, uFog * 0.5f, vBubbleFlag ) );
    oColor        = vec4( pow( color, vec3(1.1) ), 0.75 );
}