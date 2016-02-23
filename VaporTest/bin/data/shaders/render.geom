#version 150

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

uniform mat4 modelViewProjectionMatrix;

uniform float pointSize;

//in vec3 vPosition[1];
in float vCellSize[1];
in float vDensity[1];
in float vDebug[1];

out float gDensity;
out float gDebug;

void main()
{
    float cellSize = vCellSize[0] * pointSize;
	vec3 p0 = gl_in[0].gl_Position.xyz - vec3(cellSize * 0.5, cellSize * 0.5, cellSize * 0.5);

    gDensity = vDensity[0];
    gDebug = vDebug[0];

    // Front
    gl_Position = modelViewProjectionMatrix * vec4(p0, 1.0);
    EmitVertex();

	gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, 0), 1.0);
	EmitVertex();
	
	gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, 0), 1.0);
	EmitVertex();
	
	gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, 0), 1.0);
	EmitVertex();

    EndPrimitive();

    // Top
    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, cellSize), 1.0);
    EmitVertex();

    EndPrimitive();

    // Back
    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, 0, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, cellSize), 1.0);
    EmitVertex();

    EndPrimitive();

    // Bottom
    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, 0, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, 0, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, 0), 1.0);
    EmitVertex();

    EndPrimitive();

    // Left
    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, 0, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, 0, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(0, cellSize, 0), 1.0);
    EmitVertex();

    EndPrimitive();

    // Right
    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, 0, cellSize), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, 0), 1.0);
    EmitVertex();

    gl_Position = modelViewProjectionMatrix * vec4(p0 + vec3(cellSize, cellSize, cellSize), 1.0);
    EmitVertex();

    EndPrimitive();

}