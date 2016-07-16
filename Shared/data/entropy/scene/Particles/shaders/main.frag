// Filename: debugNormals.frag
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Sat Nov 14 13:50:14 2015 (-0500)
// Last-Updated: Mon Jan 25 17:05:45 2016 (-0500)

#version 330 core

#pragma include <inc/ofDefaultUniforms.glsl>

#pragma include <inc/viewData.glsl>
#pragma include <inc/clusteredShading.glsl>

// PBR
#pragma include <inc/math.glsl>
#pragma include <inc/toneMapping.glsl>
#pragma include <inc/pbr.glsl>

in vec4 vVertex;
in vec3 vNormal;
in vec2 vTexCoord0;

in vec3 vVertex_ws;
in vec3 vNormal_ws;
in vec3 vEyeDir_ws;

in float vLinearFragDepth;

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
    vec3 V = normalize( -vVertex.xyz ); // view position  

    float NoV = saturate( dot( N, V ) ) + EPSILON;

    vec3 specularContrib = vec3( 0.0, 0.0, 0.0 );
    vec3 diffuseContrib = vec3( 0.0, 0.0, 0.0 );

    int lightIndexOffset = 0;
    int pointLightCount = 0;
    GetLightOffsetAndCount( gl_FragCoord.xy, vVertex.z, lightIndexOffset, pointLightCount );

    vec3 diffuseResult = vec3( 0.0f );
    vec3 specularResult = vec3( 0.0f );

    for ( int i = 0; i < pointLightCount; ++i )
    {
        PointLight light = GetPointLight( lightIndexOffset++ );
        CalcPointLight( light, vVertex.xyz, N, V, NoV, fRoughness, specularColor, diffuseResult, specularResult );

        diffuseContrib += diffuseResult;
        specularContrib += specularResult;
    }

    for ( int i = 0; i < directionalLightCount; ++i )
    {
        DirectionalLight light = directionalLights[ i ];
        CalcDirectionalLight( light, vVertex.xyz, N, V, NoV, fRoughness, specularColor, diffuseResult, specularResult );

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
