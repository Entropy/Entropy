#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform	samplerBuffer uTexData;

in vec4 position;
in vec4 color;
//in float mass;
//in float starFormationRate;

//flat out int vEnabled;
//flat out int vID;
//flat out int vCell;

//out float vAlpha;
out vec4 vColor;

void main()
{
	int x = gl_InstanceID * 4;
	
	mat4 transformMatrix = mat4( 
		texelFetch(uTexData, x + 0),
		texelFetch(uTexData, x + 1),
		texelFetch(uTexData, x + 2), 
		texelFetch(uTexData, x + 3)
	);
	
	//vec3 origin = vec3(transformMatrix[3][0], transformMatrix[3][1], transformMatrix[3][2]);

	gl_Position = modelViewProjectionMatrix * transformMatrix * position;
	vColor = color;
}
