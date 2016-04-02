#version 150

// App uniforms and attributes
uniform sampler2DRect uPrevBuffer;
uniform sampler2DRect uCurrBuffer;

uniform float uDamping;

in vec2 vTexCoord;

out vec4 vFragColor;

void main()
{
    vec2 offset[4];
    offset[0] = vec2(-1.0,  0.0);
    offset[1] = vec2( 1.0,  0.0);
    offset[2] = vec2( 0.0,  1.0);
    offset[3] = vec2( 0.0, -1.0);
    
    //  Grab the information arround the active pixel.
    //
    //      [3]
    //
    //  [0]  st  [1]
    //
    //      [2]
    vec3 sum = vec3(0.0);
    for (int i = 0; i < 4 ; ++i) {
        sum += texture(uPrevBuffer, vTexCoord + offset[i]).rgb;
    }
    
    //  Make an average and substract the center value.
    sum = (sum / 2.0) - texture(uCurrBuffer, vTexCoord).rgb;
    sum *= uDamping;

    vFragColor = vec4(sum, 1.0);
}
