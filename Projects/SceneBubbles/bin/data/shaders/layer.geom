#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// App uniforms and attributes
uniform int uLayer;

in vec2 vTexCoord[3];

out vec2 gTexCoord;

void main() 
{
	for (int i = 0; i < 3; i++) 
	{
		gl_Position = gl_in[i].gl_Position;
		gTexCoord = vTexCoord[i];
		gl_Layer = uLayer;
		EmitVertex();
	}
	EndPrimitive();
}
