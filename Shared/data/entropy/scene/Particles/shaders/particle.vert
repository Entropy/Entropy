/*
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
	mat4 normalMatrix = transpose(inverse(transformMatrix));
    
    vPosition = transformMatrix * position;
    vNormal = normalize((normalMatrix * vec4(normal, 0.0)).xyz);
    vTexCoord = texcoord;

	 // Cube map vectors, in world space.
    vec4 eyeDir = vPosition - vec4(0.0, 0.0, 0.0, 1.0);
    mat4 inverseTransformMatrix = inverse(transformMatrix);
	vPosition_ws = (inverseTransformMatrix * vPosition).xyz;
    vEyeDir_ws = (inverseTransformMatrix * eyeDir).xyz;
    vNormal_ws = (inverseTransformMatrix * vec4(vNormal, 0.0)).xyz;
    
	gl_Position = projectionMatrix * vPosition;
}
*/
#version 150

#define MAX_LIGHTS 8

uniform samplerBuffer uOffsetTex;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
//uniform vec4 globalColor;

uniform mat4 viewMatrix;
uniform int numLights;

in vec4  position;
in vec3  normal;
in vec2  texcoord;

//out vec4 positionVarying;
//out vec3 normalVarying;
out vec2 texCoordVarying;

//out vec4 colorVarying;
out vec3 v_normalVarying;
out vec4 v_positionVarying;


void main() 
{
    int idx = gl_InstanceID * 4;
    mat4 transform = mat4( 
        texelFetch( uOffsetTex, idx ),
        texelFetch( uOffsetTex, idx+1 ),
        texelFetch( uOffsetTex, idx+2 ), 
        texelFetch( uOffsetTex, idx+3 )
    );
    mat4 mvMatrix = modelViewMatrix * transform;

    mat3 normalMatrix = transpose(inverse(mat3(mvMatrix)));
    
    //normalVarying = normal;
    //positionVarying = position;
    texCoordVarying = texcoord;
    //colorVarying = globalColor;
    
    v_normalVarying = normalize(vec3(normalMatrix * normal));
    v_positionVarying = mvMatrix * position;
    
	gl_Position = projectionMatrix * v_positionVarying;
}