#version 150

// App uniforms and attributes
uniform sampler2D uTexColor;

//uniform sampler2DRect uTexMask;
//uniform vec2 uMaskDims;

uniform sampler2DArray uTexMask;
uniform vec3 uMaskDims;

uniform float uAlphaBase;
uniform float uMaskMix;

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

	//vec4 maskColor = texture(uTexMask, vTexCoord * uMaskDims);
	vec3 maskCoord = vPosition_ws.xyz / vec3(800.0) + vec3(0.5);
	maskCoord.y = 1.0 - maskCoord.y;
	vec4 maskColor = texture(uTexMask, maskCoord);
	
	float alpha = clamp(mix(uAlphaBase, luminance(maskColor.rgb), uMaskMix), 0.0, 1.0);

	fragColor = texColor;
	fragColor = vec4(texColor.rgb, alpha);
}
