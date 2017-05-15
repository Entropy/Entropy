#version 330

// OF default uniforms and attributes
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;

in vec4 position;
in vec3 texcoord;

// App uniforms and attributes
out vec4 vPosition;
out vec3 vTexCoord;
out vec3 vCameraPosition;

void main()
{
	vPosition = modelViewProjectionMatrix * position;
	vTexCoord = texcoord;
	vCameraPosition = (inverse(modelViewMatrix) * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

	gl_Position = vPosition;
}
