#version 400

#define MAX_LIGHTS 8

uniform mat4 viewMatrix;

struct Light{
    vec3 position;
    vec4 color;
    float intensity;
    float radius;
};

uniform Light lights[MAX_LIGHTS];
uniform int numLights;
uniform float roughness;
uniform vec3 particleColor;

in vec4 colorVarying;
in vec2 texCoordVarying;

in vec3 v_normalVarying;
in vec4 v_positionVarying;

out vec4 fragColor;

float BechmannDistribution(float d, float m)
{
    float d2 = d * d;
    float m2 = m * m;
    return exp((d2 - 1.0) / (d2 * m2)) / (m2 * d2 * d2);
}

float CookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness)
{
    vec3 l = normalize(lightDirection);
    vec3 n = normalize(surfaceNormal);
    
    vec3 v = normalize(viewDirection);
    vec3 h = normalize(l + v);
    
    float hn = dot(h, n);
    float ln = dot(l, n);
    float lh = dot(l, h);
    float vn = dot(v, n);
    
    float f = 0.02 + pow(1.0 - dot(v, h), 5.0) * (1.0 - 0.02);
    float d = BechmannDistribution(hn, roughness);
    float t = 2.0 * hn / dot(v, h);
    float g = min(1.0, min(t * vn, t * ln));
    float m = 3.14159265 * vn * ln;
    float spec = max(f * d * g / m, 0.0);
    return spec;
}

float Falloff(float dist, float lightRadius)
{
    float att = clamp(1.0 - dist * dist / (lightRadius * lightRadius), 0.0, 1.0);
    att *= att;
    return att;
}

vec3 CalcPointLight(vec4 v_positionVarying, vec3 v_normalVarying, vec3 color, Light light)
{
    vec3 s = normalize(light.position - v_positionVarying.xyz);
    float lambert = max(dot(s, v_normalVarying), 0.0);
    float falloff = Falloff(length(light.position - v_positionVarying.xyz), light.radius);
    float specular = CookTorranceSpecular(s, -normalize(v_positionVarying.xyz), v_normalVarying, roughness);
    return light.intensity * (color.rgb * light.color.rgb * lambert * falloff + color.rgb * light.color.rgb * specular * falloff);
}

void main (void)
{
    vec3 color = vec3(0.0);
    vec3 light = vec3(0.0);
    for(int i = 0; i < numLights; i++)
	{
		//color += CalcPointLight(v_positionVarying, v_normalVarying, colorVarying.xyz, lights[i]);
		color += CalcPointLight(v_positionVarying, v_normalVarying, vec3(1.0), lights[i]);
    }
    fragColor = vec4(particleColor, 1.0) * vec4(color, 1.0);
}