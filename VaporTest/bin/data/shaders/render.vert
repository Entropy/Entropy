#version 150

uniform mat4 modelViewProjectionMatrix;

in vec4 position;
in float density;

uniform samplerBuffer uTransform;

uniform float uDebugMin;
uniform float uDebugMax;

out float vDensity;
out float vDebug;

void main()
{
    // Get the transform from the sampler.
    vec4 transform = texelFetch(uTransform, gl_InstanceID);
    mat4 transformMatrix = mat4(transform.w, 0.0,         0.0,         0.0,
                                0.0,         transform.w, 0.0,         0.0,
                                0.0,         0.0,         transform.w, 0.0,
                                transform.x, transform.y, transform.z, 1.0);

    gl_Position = modelViewProjectionMatrix * transformMatrix * position;

    if (uDebugMin <= position.z && position.z < uDebugMax) {
        vDebug = 1.0;
    }
    else {
        vDebug = 0.0;
    }

    vDensity = density;
}
