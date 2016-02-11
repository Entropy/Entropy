#version 150

uniform mat4 modelViewProjectionMatrix;

uniform float pointSize;

in vec4 position;
in float cellSize;
in float density;

out float vDensity;

void main()
{
	gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = pointSize * cellSize;
	
    vDensity = density;
}





