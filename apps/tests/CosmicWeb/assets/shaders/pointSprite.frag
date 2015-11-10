#version 150

#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D	uTexture;
uniform vec3   		uLowGradient;
uniform vec3   		uHighGradient;
uniform float   uAlphaScale;

in float          	vDistance;
in float			vDensity;

out vec4 			oColor;

void main() {
   	vec3 rgb 		= mix( uHighGradient, uLowGradient, vDensity );
	float dist 		= 1.0 - length( 2.0 * gl_PointCoord.xy - vec2(1.0) );
	dist 			= 	0.4 * smoothstep( 0.0, 1.0, dist ) + 
						0.2 * smoothstep( 0.85, 1.0, dist );
    float fogAmount = exp( -vDistance * 0.0008 );
    oColor			= vec4( rgb * dist, fogAmount * dist ) * uAlphaScale;
}