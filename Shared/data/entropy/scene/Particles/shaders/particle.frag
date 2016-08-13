/*
#version 330 core

#pragma include <inc/ofDefaultUniforms.glsl>

#pragma include <inc/viewData.glsl>
#pragma include <inc/clusteredShading.glsl>

// PBR
#pragma include <inc/math.glsl>
#pragma include <inc/toneMapping.glsl>
#pragma include <inc/pbr.glsl>

in vec4 vPosition;
in vec3 vNormal;
in vec2 vTexCoord;

in vec3 vPosition_ws;
in vec3 vNormal_ws;
in vec3 vEyeDir_ws;

out vec4 fragColor;

void main(void)
{
    vec3 baseColor = uBaseColor.rgb;
    float fMetalness = uMetallic;
    float fRoughness = uRoughness;

    // metallic is fully black base color
    vec3 diffuseColor = baseColor * ( 1.0f - fMetalness );

    // get specular color from the base color based on specular level and how metallic the material is 
    // 0.04 is specular color for non-metallic surfaces, but UE4/Disney uses 0.08
    const vec3 dielectricColor = vec3( 0.08f );
    vec3 specularColor = mix( dielectricColor, baseColor, fMetalness );

    vec3 N = normalize( vNormal ); // normal
    vec3 V = normalize( -vPosition.xyz ); // view position  

    float NoV = saturate( dot( N, V ) ) + EPSILON;

    vec3 specularContrib = vec3( 0.0, 0.0, 0.0 );
    vec3 diffuseContrib = vec3( 0.0, 0.0, 0.0 );

    int lightIndexOffset = 0;
    int pointLightCount = 0;
    GetLightOffsetAndCount( gl_FragCoord.xy, vPosition.z, lightIndexOffset, pointLightCount );

    vec3 diffuseResult = vec3( 0.0f );
    vec3 specularResult = vec3( 0.0f );

    for ( int i = 0; i < pointLightCount; ++i )
    {
        PointLight light = GetPointLight( lightIndexOffset++ );
        CalcPointLight( light, vPosition.xyz, N, V, NoV, fRoughness, specularColor, diffuseResult, specularResult );

        diffuseContrib += diffuseResult;
        specularContrib += specularResult;
    }

    for ( int i = 0; i < directionalLightCount; ++i )
    {
        DirectionalLight light = directionalLights[ i ];
        CalcDirectionalLight( light, vPosition.xyz, N, V, NoV, fRoughness, specularColor, diffuseResult, specularResult );

        diffuseContrib += diffuseResult;
        specularContrib += specularResult;    
     }

    const int NUM_MIP_LEVELS = 7;

    // Image based lighting
    vec3 normal_ws = normalize( vNormal_ws ); 
    vec3 reflect_ws = normalize( reflect( vEyeDir_ws, normal_ws ) );

    diffuseContrib += CalcIBLDiffuse( uIrradianceMap, normal_ws, ambientIntensity );
    specularContrib += CalcIBLSpecular( uRadianceMap, NUM_MIP_LEVELS, reflect_ws, NoV, fRoughness, ambientIntensity );
 
    diffuseContrib *= diffuseColor;
    specularContrib *= specularColor;

    diffuseContrib = mix( diffuseContrib, uEmissiveColor.rgb, uEmissiveIntensity ); // emissive

    vec3 color = diffuseContrib + specularContrib;

    // tonemap function requires exposure corrected color
    color = tonemapUncharted2( color * uExposure );

    // Correct for white input level
    const float whiteInputLevel = 2.0f;
    vec3 whiteScale = 1.0f / tonemapUncharted2( vec3( whiteInputLevel ) );
    color = color * whiteScale;

    fragColor = vec4( linearToGamma( color, uGamma ), uBaseColor.a );
}
*/
#version 150

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

//in vec4 colorVarying;
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