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

uniform vec3 uDims;

in vec2 gTexCoord;

out vec4 fragColor;

void main()
{
	vec3 fragPos = vec3(gTexCoord, gl_Layer);

	if (abs(distance(uBurst.pos, fragPos) - uBurst.radius) <= uBurst.thickness) 
	{
		fragColor = globalColor;
	}
	else {
		fragColor = vec4(0.0);
	}
}
