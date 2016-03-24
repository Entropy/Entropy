#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;

uniform float debugMin;
uniform float debugMax;

in vec4 position;
in float cellSize;
in float density;

//out vec4 vPosition;
out float vCellSize;
out float vDensity;
out float vDebug;

void main()
{
//    vec4 eyeCoord = modelViewMatrix * position;
    gl_Position = position;

//    float dist = sqrt(eyeCoord.x * eyeCoord.x + eyeCoord.y * eyeCoord.y + eyeCoord.z * eyeCoord.z);
//    float attenuation = 600.0 / dist;
//    gl_PointSize = pointSize * cellSize * 2 * attenuation;

    if (debugMin <= position.z && position.z < debugMax) {
        vDebug = 1.0;
    }
    else {
        vDebug = 0.0;
    }

    vCellSize = cellSize;
    vDensity = density;
}
