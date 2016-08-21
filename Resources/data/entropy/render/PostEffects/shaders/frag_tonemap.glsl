#version 330
out vec4 fragColor;

uniform sampler2D tex0;
uniform sampler2D blurred1;
uniform float brightness;
uniform float gamma;
uniform float contrast;
uniform float tonemap_type;
uniform float exposureBias = 2.0f;

in vec2 f_texcoord;



// handy value clamping to 0 - 1 range
float saturate(in float value)
{
    return clamp(value, 0.0, 1.0);
}

// handy value clamping to 0 - 1 range
vec3 saturate(in vec3 value)
{
    return clamp(value, vec3(0.0), vec3(1.0));
}

float lumf(vec3 _rgb)
{
    return dot(vec3(0.2126729, 0.7151522, 0.0721750), _rgb);
}

float toGamma(float _r)
{
	return pow(abs(_r), 1.0/gamma);
}

vec3 toGamma(vec3 _rgb)
{
	return pow(abs(_rgb), vec3(1.0/gamma) );
}

vec4 toGamma(vec4 _rgba)
{
	return vec4(toGamma(_rgba.xyz), _rgba.w);
}

vec3 toGammaAccurate(vec3 _rgb)
{
	vec3 lo  = _rgb * 12.92;
	vec3 hi  = pow(abs(_rgb), vec3(1.0/2.4) ) * 1.055 - 0.055;
	vec3 rgb = mix(hi, lo, vec3(lessThanEqual(_rgb, vec3(0.0031308) ) ) );
	return rgb;
}

vec4 toGammaAccurate(vec4 _rgba)
{
	return vec4(toGammaAccurate(_rgba.xyz), _rgba.w);
}

vec3 toReinhard1(vec3 _rgb)
{
	return toGamma(_rgb/(_rgb+vec3(1.0) ) );
}

vec4 toReinhard1(vec4 _rgba)
{
	return vec4(toReinhard1(_rgba.xyz), _rgba.w);
}

vec3 toFilmic(vec3 _rgb)
{
	_rgb = max(vec3(0.0), _rgb - 0.004);
	_rgb = (_rgb*(6.2*_rgb + 0.5) ) / (_rgb*(6.2*_rgb + 1.7) + 0.06);
	return _rgb;
}

vec4 toFilmic(vec4 _rgba)
{
	return vec4(toFilmic(_rgba.xyz), _rgba.w);
}

vec3 toAcesFilmic(vec3 _rgb)
{
	// Reference:
	// ACES Filmic Tone Mapping Curve
	// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
	float aa = 2.51f;
	float bb = 0.03f;
	float cc = 2.43f;
	float dd = 0.59f;
	float ee = 0.14f;
	return saturate( (_rgb*(aa*_rgb + bb) )/(_rgb*(cc*_rgb + dd) + ee) );
}

vec4 toAcesFilmic(vec4 _rgba)
{
	return vec4(toAcesFilmic(_rgba.xyz), _rgba.w);
}

vec3 toReinhard2(vec3 _rgb, float _whiteSq)
{
    float lumRgb = lumf(_rgb);
    float lumScale = (lumRgb * (1.0 + lumRgb / _whiteSq))/(1.0 + lumRgb);
    return toGamma(_rgb*lumScale/lumRgb);
}

const float a = 0.22; // Shoulder strength
const float b = 0.30; // Linear strength
const float c = 0.10; // Linear angle
const float d = 0.20; // Toe Strength
const float e = 0.01; // Toe numerator
const float f = 0.30; // Toe denominator
					  // e/f Toe angle
const float w = 11.2; // Linear white point value

/*uniform float exposureBias;
uniform float a; // Shoulder strength
uniform float b; // Linear strength
uniform float c; // Linear angle
uniform float d; // Toe Strength
uniform float e; // Toe numerator
uniform float f; // Toe denominator
					  // e/f Toe angle
uniform float w = 11.2; // Linear white point value*/

vec3 toUncharted2(vec3 _x)
{
    return ((_x*(a*_x+c*b)+d*e)/(_x*(a*_x+b)+d*f))-e/f;
}

vec3 toUncharted2(float _x)
{
    vec3 x = vec3(_x);
	return toUncharted2(x);
    /*float a = 0.22;
    float b = 0.30;
    float c = 0.10;
    float d = 0.20;
    float e = 0.01;
    float f = 0.30;
    float w = 11.2;
    return ((x*(a*x+c*b)+d*e)/(x*(a*x+b)+d*f))-e/f;*/
}

vec3 sepia(vec3 _rgb)
{
	vec3 color;
	color.x = dot(_rgb, vec3(0.393, 0.769, 0.189) );
	color.y = dot(_rgb, vec3(0.349, 0.686, 0.168) );
	color.z = dot(_rgb, vec3(0.272, 0.534, 0.131) );
	return color;
}

vec4 sepia(vec4 _rgba)
{
	return vec4(sepia(_rgba.xyz), _rgba.w);
}

vec3 tonemap(vec3 _rgb, float _whiteSq, float u_toneMapping)
{
    if (0.0 == u_toneMapping) // Linear
    {
        return _rgb;
    }
    if (1.0 == u_toneMapping) // Gamma
    {
        return toGamma(_rgb);
    }
    else if (2.0 == u_toneMapping) // Reinhard
    {
        return toReinhard1(_rgb);
    }
    else if (3.0 == u_toneMapping) // Reinhard2
    {
        return toReinhard2(_rgb, _whiteSq);
    }
    else if (4.0 == u_toneMapping) // Filmic
    {
        return toFilmic(_rgb);
    }
    else if (5.0 == u_toneMapping) // Filmic
    {
        return toAcesFilmic(_rgb);
    }
    else //if (5.0 == u_toneMapping). // Uncharted2
    {
        vec3 curr = exposureBias*toUncharted2(_rgb);

        vec3 whiteScale = 1.0f/toUncharted2(w);

        _rgb = curr*whiteScale;

        return toGamma(_rgb);
    }
}

void main(){
    vec3 color_hdr = texture(tex0, vec2(f_texcoord.x, 1.0-f_texcoord.y)).rgb;
    vec3 color_bloom = texture(blurred1, f_texcoord).rgb;
    vec3 rgb = tonemap(color_hdr + color_bloom, 1.0, tonemap_type);

    // Apply contrast.
    rgb = mix(vec3(0.18), rgb, contrast);

    // Apply brightness.
    rgb = max(vec3(0.0), rgb + vec3(brightness));

    fragColor = vec4(rgb, 1.0);
}
