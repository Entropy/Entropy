#version 330

uniform samplerBuffer photonPosnTexture;

// ping pong inputs
uniform sampler2DRect particles0;
uniform sampler2DRect particles1;

uniform vec3 mouse;
uniform float elapsedTime;
uniform float frameTime;

in vec2 texCoordVarying;

layout(location = 0) out vec4 posOut;
layout(location = 1) out vec4 velOut;

void main()
{
    vec4 pos = texture(particles0, texCoordVarying.st);
    vec4 vel = texture(particles1, texCoordVarying.st);
 
    // last term pos.w is age to this is where decay happens
    pos += vec4(frameTime * vel.xyz, -0.03);
    
    int photonIdx = int(floor(texCoordVarying.s));
    
    vec4 photonPos = texelFetch( photonPosnTexture, photonIdx );
    
    // change to step function to make more efficient
    if (pos.w < 0.0) pos = vec4(photonPos.xyz, 1.0);//vec4(photons[photonIdx], 1.0);//vec4(mouse, 1.0);
    
    posOut = pos;
    velOut = vel;
}