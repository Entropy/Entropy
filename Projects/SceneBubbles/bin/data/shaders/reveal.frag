#version 430

#define USE_TEX_ARRAY 1

// App uniforms and attributes
uniform sampler2D uTexColor;

#if USE_TEX_ARRAY
uniform sampler2DArray uTexMask;
#else
uniform sampler3D uTexMask;
#endif
uniform vec3 uMaskDims;
uniform float uVolSize;

uniform float uAlphaBase;
uniform float uMaskMix;

uniform vec4 uTintColor;

in vec4 vPosition_ws;
in vec2 vTexCoord;

out vec4 fragColor;

float luminance(vec3 rgb)
{
	return dot(vec3(0.2126729, 0.7151522, 0.0721750), rgb);
}

void main()
{
	vec4 texColor = texture(uTexColor, vTexCoord);

	const vec3 halfVec = vec3(0.5);
	vec3 maskCoord = vPosition_ws.xyz / vec3(uVolSize) + vec3(0.5);
#if USE_TEX_ARRAY
	maskCoord.z *= uMaskDims.z;
#endif
	maskCoord.y = 1.0 - maskCoord.y;
	vec4 maskColor = texture(uTexMask, maskCoord);
	
	float alpha = clamp(uAlphaBase + luminance(maskColor.rgb) * uMaskMix, 0.0, 1.0);

	vec3 rgbColor = texColor.rgb * uTintColor.rgb;
	fragColor = vec4(rgbColor, alpha);
}
