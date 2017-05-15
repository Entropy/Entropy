#version 430

#define USE_TEX_ARRAY 1

// App uniforms and attributes
#if USE_TEX_ARRAY
uniform sampler2DArray uPrevBuffer;
uniform sampler2DArray uCurrBuffer;
#else
uniform sampler3D uPrevBuffer;
uniform sampler3D uCurrBuffer;
#endif
uniform vec3 uDims;
uniform float uDamping;

in vec2 gTexCoord;

out vec4 fragColor;

void main()
{
	const vec3 offset[6] = vec3[](
		vec3(-1.0,  0.0,  0.0),
		vec3( 1.0,  0.0,  0.0),
		vec3( 0.0, -1.0,  0.0),
		vec3( 0.0,  1.0,  0.0),
		vec3( 0.0,  0.0, -1.0),
		vec3( 0.0,  0.0,  1.0));
	
	//  Grab the information arround the active pixel.
	//
	//      [3]
	//
	//  [0]  st  [1]
	//
	//      [2]
#if USE_TEX_ARRAY
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer);
#else
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer / uDims.z);
#endif
	vec4 sum = vec4(0.0);
	for (int i = 0; i < 6; ++i) 
	{
		vec3 texOffset = offset[i] / vec3(uDims.xy, 1.0);
		sum += texture(uPrevBuffer, texCoord + texOffset);
	}
	
	//  Make an average and substract the center value.
	sum = (sum / 3.0) - texture(uCurrBuffer, texCoord);
	sum *= uDamping;

	fragColor = sum;

#if USE_TEX_ARRAY
	// TODO: Interpolate between layers
	//int prevLayer = int(gl_Layer);
	//int nextLayer = prevLayer + 1;
	//float amt = fract(uLayer);
	//fragColor = mix(texture(uTexArray, vec3(vTexCoord, prevLayer)), 
	//				 texture(uTexArray, vec3(vTexCoord, nextLayer)), 
	//				 amt);
#endif
}
