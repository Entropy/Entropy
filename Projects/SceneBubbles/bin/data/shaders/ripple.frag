#version 150

// App uniforms and attributes
uniform sampler2DRect uPrevBuffer;
uniform sampler2DRect uCurrBuffer;

uniform float uDamping;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	const vec2 offset[4] = vec2[](
		vec2(-1.0,  0.0),
		vec2( 1.0,  0.0),
		vec2( 0.0, -1.0),
		vec2( 0.0,  1.0));
    
    //  Grab the information arround the active pixel.
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]
    vec4 sum = vec4(0.0);
    for (int i = 0; i < 4 ; ++i) {
        sum += texture(uPrevBuffer, vTexCoord + offset[i]);
    }
    
    //  Make an average and substract the center value.
    sum = (sum / 2.0) - texture(uCurrBuffer, vTexCoord);
    sum *= uDamping;

	fragColor = vec4(sum);
}
