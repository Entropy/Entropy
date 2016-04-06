#version 430

// OF default uniforms and attributes
uniform vec4 globalColor;

// App uniforms and attributes
uniform struct Burst 
{
    vec3 pos;
    float radius;
    float thickness;
} uBurst;

in vec2 gTexCoord;

out vec4 fFragColor;

void main()
{
    vec3 fragPos = vec3(gTexCoord, gl_Layer);
    if (abs(distance(uBurst.pos, fragPos) - uBurst.radius) <= uBurst.thickness) {
        fFragColor = globalColor;
    }
    else {
        fFragColor = vec4(0.0);
    }
}
