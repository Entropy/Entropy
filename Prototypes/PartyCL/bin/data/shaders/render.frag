#version 150

// OF default uniforms and attributes
uniform vec4 globalColor;

// App uniforms and attributes
uniform sampler2D uPointTexture;

out vec4 vFragColor;

void main()
{
    vec4 color = (0.6 + 0.4 * globalColor) * texture(uPointTexture, gl_PointCoord);
    vFragColor = color * mix(vec4(0.0, 0.2, 0.2, color.a), vec4(0.2, 0.7, 0.7, color.a), color.a);
}
