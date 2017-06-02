#version 330

// addressed by OF
uniform mat4 modelViewProjectionMatrix;

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec2 texcoord;
// end addressed by OF

out vec2 vTexCoord;

void main()
{
    gl_Position = modelViewProjectionMatrix * position;
    vTexCoord = texcoord;
}
