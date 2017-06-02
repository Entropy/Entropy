#version 330

// addressed by OF
uniform vec4 globalColor;
// end addressed by OF

uniform sampler2D tex0;
uniform float brightness;
uniform float contrast;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
    vec4 color = texture(tex0, vTexCoord) * globalColor;

    // Apply contrast.
    color.rgb = mix(vec3(0.18), color.rgb, contrast);

    // Apply brightness.
    color.rgb = max(vec3(0.0), color.rgb + vec3(brightness));

    fragColor = color;
}
