#version 430

#define USE_TEX_ARRAY 1

// App uniforms and attributes
#if USE_TEX_ARRAY
uniform sampler2DArray uCopyBuffer;
#else
uniform sampler3D uCopyBuffer;
#endif
uniform vec3 uDims;

in vec2 gTexCoord;

out vec4 fragColor;

void main()
{
#ifdef USE_TEX_ARRAY
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer);
#else
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer / uDims.z);
#endif
	fragColor = texture(uCopyBuffer, texCoord);
}
