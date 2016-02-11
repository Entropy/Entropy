#version 150

uniform mat4 modelViewProjectionMatrix;

uniform float pointSize;

uniform float debugMin;
uniform float debugMax;

in vec4 position;
in float cellSize;
in float density;

out float vDensity;
out float vDebug;

void main()
{
	gl_Position = modelViewProjectionMatrix * position;
    gl_PointSize = pointSize * cellSize;

    if (debugMin <= position.z && position.z < debugMax) {
        vDebug = 1.0;
    }
    else {
        vDebug = 0.0;
    }

    vDensity = density;
}
