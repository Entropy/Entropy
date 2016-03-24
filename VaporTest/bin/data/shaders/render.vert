#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in float density;

uniform samplerBuffer uTex;

uniform float uDebugMin;
uniform float uDebugMax;

out float vDensity;
out float vDebug;

void main()
{
    // Get the transform from the sampler.
    int coord = gl_InstanceID * 4;
    mat4 transformMatrix = mat4(texelFetch(uTex, coord + 0),
                                texelFetch(uTex, coord + 1),
                                texelFetch(uTex, coord + 2),
                                texelFetch(uTex, coord + 3));

    gl_Position = modelViewProjectionMatrix * transformMatrix * position;

    if (uDebugMin <= position.z && position.z < uDebugMax) {
        vDebug = 1.0;
    }
    else {
        vDebug = 0.0;
    }

    vDensity = density;
}
