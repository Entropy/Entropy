#version 330

in vec4 f_color;
in float f_distanceToCamera;
in float f_distanceToCenter;

uniform float fogMaxDistance;
uniform float fogMinDistance;
uniform float fogStartDistance;
uniform float fogPower;
uniform float fadeEdge0;
uniform float fadeEdge1;
uniform float fadePower;

uniform float screenW;
uniform float screenH;
uniform float wobblyClip;

uniform sampler2D minDepthMask;
uniform sampler2D maxDepthMask;

out vec4 fragColor;
#define FOG_ENABLED 0
#define SPHERICAL_CLIP 0


float fog(float dist, float startDist, float minDist, float maxDist, float power) {
	return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
}

void main(void)
{
	vec2 texcoord = vec2(gl_FragCoord.x / screenW, gl_FragCoord.y / screenH);
	texcoord.y = 1 - texcoord.y;
	float minDepth = texture(minDepthMask, texcoord).r;
	float maxDepth = texture(maxDepthMask, texcoord).r;
	if(wobblyClip<1 || (gl_FragCoord.z<minDepth && gl_FragCoord.z>maxDepth)){
	    fragColor = f_color;
    #if FOG_ENABLED
		fragColor *= fog(f_distanceToCamera, fogStartDistance, fogMinDistance, fogMaxDistance, fogPower);
    #endif
    #if SPHERICAL_CLIP
		float sphere = 1 - pow(smoothstep(fadeEdge0, fadeEdge1, f_distanceToCenter), fadePower);
		fragColor.a *= sphere;
    #endif
	}else{
		discard;
	}
}
