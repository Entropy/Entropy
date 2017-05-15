#version 150

// OF default uniforms and attributes
uniform vec4 globalColor;

// App uniforms and attributes
in float vAlpha;

out vec4 fragColor;

void main()
{
	fragColor = vec4(globalColor.rgb, globalColor.a * vAlpha);
}
