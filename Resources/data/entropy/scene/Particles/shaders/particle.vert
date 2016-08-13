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