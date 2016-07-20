#version 150

#define MAX_LIGHTS 8

uniform samplerBuffer uOffsetTex;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat3 normalMatrix;
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
    mat3 normalMatrix = transpose(inverse(mat3(modelViewMatrix)));
    
    //normalVarying = normal;
    //positionVarying = position;
    texCoordVarying = texcoord;
    //colorVarying = globalColor;
    
    v_normalVarying = normalize(vec3(normalMatrix * normal));
    v_positionVarying = modelViewMatrix * position;
    
	gl_Position = projectionMatrix * v_positionVarying;
}