#pragma once

#include "ofMain.h"

#include "lb/gl/CubeMapTexture.h"

struct PBRMaterial
{
    static const uint8_t    UPDATE_BASE_COLOR = 1;
    static const uint8_t    UPDATE_METALLIC = 1 << 1;
    static const uint8_t    UPDATE_ROUGHNESS = 1 << 2;
    static const uint8_t    UPDATE_ALBEDO_MAP = 1 << 3;
    static const uint8_t    UPDATE_NORMAL_MAP = 1 << 4;
    static const uint8_t    UPDATE_METALNESS_MAP = 1 << 5;
    static const uint8_t    UPDATE_ROUGHNESS_MAP = 1 << 6;

    uint32_t          updateFlags;

    std::string       name;

    ofFloatColor	  baseColor;
    float       	  metallic;
    float       	  roughness;
    ofFloatColor	  emissiveColor;
    float        	  emissiveIntensity;

    ofPtr<ofTexture> texAlbedo;
    ofPtr<ofTexture> texNormal;
    ofPtr<ofTexture> texMetalness;
    ofPtr<ofTexture> texRoughness;

    PBRMaterial()
        : updateFlags( 0 )
        , name( "" )
        , baseColor( 0.0f, 0.0f, 0.0f, 1.0f )
        , metallic( 0.0f )
        , roughness( 0.0f )
        , emissiveColor( 0.0f, 0.0f, 0.0f, 1.0f )
        , emissiveIntensity( 0.0f )
        , texAlbedo( nullptr )
        , texNormal( nullptr )
        , texMetalness( nullptr )
        , texRoughness( nullptr )
    {
    }

    void ClearUpdateFlags()
    {
        updateFlags = 0;
    }

    void SetName( const std::string& _name )
    {
        name = _name;
    }

    void SetBaseColor( const ofFloatColor& _color )
    {
        baseColor = _color;
        updateFlags |= PBRMaterial::UPDATE_BASE_COLOR;
    }

    void SetMetallic( float _metallic )
    {
        metallic = std::min( std::max( _metallic, 0.001f ), 1.0f );
        updateFlags |= PBRMaterial::UPDATE_METALLIC;
    }

    void SetRoughness( float _roughness )
    {
        roughness = std::min( std::max( _roughness, 0.001f ), 1.0f );
        updateFlags |= PBRMaterial::UPDATE_ROUGHNESS;
    }

    void SetEmissiveColor( const ofFloatColor& _color )
    {
        emissiveColor = _color;
        updateFlags |= PBRMaterial::UPDATE_BASE_COLOR;
    }

    void SetEmissiveIntensity( float _intensity )
    {
        emissiveIntensity = std::min( std::max( _intensity, 0.0f ), 1.0f );
        updateFlags |= PBRMaterial::UPDATE_METALLIC;
    }

    void SetAlbedoMap( ofPtr<ofTexture> _tex )
    {
        texAlbedo = _tex;
        updateFlags |= PBRMaterial::UPDATE_ALBEDO_MAP;
    }

    void SetNormalMap( ofPtr<ofTexture> _tex )
    {
        texNormal = _tex;
        updateFlags |= PBRMaterial::UPDATE_NORMAL_MAP;
    }

    void SetMetalnessMap( ofPtr<ofTexture> _tex )
    {
        texMetalness = _tex;
        updateFlags |= PBRMaterial::UPDATE_METALNESS_MAP;
    }

    void SetRoughnessMap( ofPtr<ofTexture> _tex )
    {
        texRoughness = _tex;
        updateFlags |= PBRMaterial::UPDATE_ROUGHNESS_MAP;
    }

    void Bind( const ofShader& _shader )
    {
        _shader.setUniform4f( "uBaseColor", baseColor );
        _shader.setUniform1f( "uMetallic", metallic );
        _shader.setUniform1f( "uRoughness", roughness );
        _shader.setUniform4f( "uEmissiveColor", emissiveColor );
        _shader.setUniform1f( "uEmissiveIntensity", emissiveIntensity );
    }
};
