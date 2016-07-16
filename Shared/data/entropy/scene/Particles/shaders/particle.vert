#version 400

#pragma include <inc/ofDefaultUniforms.glsl>
#pragma include <inc/ofDefaultVertexInAttributes.glsl> 

#pragma include <inc/viewData.glsl>

uniform samplerBuffer uOffsetTex;

out vec4 vPosition;
out vec3 vNormal;
out vec2 vTexCoord;

out vec3 vPosition_ws;
out vec3 vEyeDir_ws;
out vec3 vNormal_ws;

void main() 
{
    int idx = gl_InstanceID * 4;
    mat4 transform = mat4( 
        texelFetch(uOffsetTex, idx+0),
        texelFetch(uOffsetTex, idx+1),
        texelFetch(uOffsetTex, idx+2), 
        texelFetch(uOffsetTex, idx+3)
    );
    mat4 transformMatrix = modelViewMatrix * transform;
	mat4  normalMatrix = transpose(inverse(transformMatrix));
    
    vPosition = transformMatrix * position;
    vNormal = normalize((normalMatrix * vec4(normal, 0.0)).xyz);
    vTexCoord = texcoord;

	 // Cube map vectors, in world space.
    vec4 eyeDir = vPosition - vec4( 0.0, 0.0, 0.0, 1.0);
    vPosition_ws = (viewData.inverseViewMatrix * vPosition).xyz;
    vEyeDir_ws = vec3(viewData.inverseViewMatrix * eyeDir);
    vNormal_ws = vec3(viewData.inverseViewMatrix * vec4(vNormal, 0.0));
    
	gl_Position = projectionMatrix * vPosition;
}