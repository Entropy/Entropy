#version 430

// App uniforms and attributes
uniform sampler2DArray uPrevBuffer;
uniform sampler2DArray uCurrBuffer;
uniform vec3 uDims;
uniform float uDamping;

in vec2 gTexCoord;

out vec4 fFragColor;

void main()
{
    vec3 offset[6];
    offset[0] = vec3(-1.0,  0.0,  0.0);
    offset[1] = vec3( 1.0,  0.0,  0.0);
    offset[2] = vec3( 0.0,  1.0,  0.0);
    offset[3] = vec3( 0.0, -1.0,  0.0);
	offset[4] = vec3( 0.0,  0.0, -1.0);
    offset[5] = vec3( 0.0,  0.0,  1.0);
    
    //  Grab the information arround the active pixel.
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]
	//vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer);
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer);
    vec4 sum = vec4(0.0);
    for (int i = 0; i < 6; ++i) {
		vec3 texOffset = offset[i] / vec3(uDims.xy, 1.0);
        sum += texture(uPrevBuffer, texCoord + texOffset);
    }
    
    //  Make an average and substract the center value.
    sum = (sum / 3.0) - texture(uCurrBuffer, texCoord);
    sum *= uDamping;

    fFragColor = vec4(sum.rgb, 1.0);
	fFragColor = sum;

	//int prevLayer = int(gl_Layer);
	//int nextLayer = prevLayer + 1;
	//float amt = fract(uLayer);
	//vFragColor = mix(texture(uTexArray, vec3(vTexCoord, prevLayer)), 
	//				 texture(uTexArray, vec3(vTexCoord, nextLayer)), 
	//				 amt);
}
