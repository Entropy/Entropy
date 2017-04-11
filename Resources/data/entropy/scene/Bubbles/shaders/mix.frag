#version 150

// App uniforms and attributes
uniform sampler2DRect uPrevBuffer;
uniform sampler2DRect uCurrBuffer;

uniform float uPct;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	fragColor = mix(texture(uPrevBuffer, vTexCoord), texture(uCurrBuffer, vTexCoord), uPct);
}
