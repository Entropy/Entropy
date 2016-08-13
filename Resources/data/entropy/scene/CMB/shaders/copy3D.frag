#version 430

// App uniforms and attributes
uniform sampler3D uCopyBuffer;
uniform vec3 uDims;

in vec2 gTexCoord;

out vec4 fFragColor;

void main()
{
	vec3 texCoord = vec3(gTexCoord / uDims.xy, gl_Layer);
	fFragColor = texture(uCopyBuffer, texCoord);
}
