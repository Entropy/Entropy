#pragma once

#include <math.h>

namespace lb
{
    struct CameraParams
    {
        float   gamma;
        float   exposureValue;
        float   aperture;
        float   shutterSpeed;
        float   filmISO;
        float   padding[ 2 ]; // ubo alignment
    };

    // Calculates EV (exposure value) based on aperture (f-stop), Shutter Speed (seconds), and Film ISO
    // Uses the standard formula: EV = log2( ( N^2 / t ) * 100 / ISO )
    // Where N is aperture, and t is shutter speed in seconds
    //
    // EV: number that represents a combination of shutter speed & f-number such that
    //     all combinations that yield the same exposure have the same EV value.
    //     1 EV is a standard power of 2 exposure step (referred to as a 'stop') 
    //     e.g. EV of 5 is twice the exposure as EV of 4 
    inline float CalcEVFromCameraSettings( float _fStop, float _shutterSpeedSeconds, float _filmISO )
    {
        float f2 = _fStop * _fStop;
        return log2f( ( f2 / _shutterSpeedSeconds ) * 100.0f / _filmISO );
    }

    // EV100 (EV at ISO 100)
    inline float CalcEVFromCameraSettings( float _fStop, float _shutterSpeedSeconds )
    {
        float f2 = _fStop * _fStop;
        return log2f( f2 / _shutterSpeedSeconds );
    }

    // Calculates EV (exposure value) from known luminance value
    inline float CalcEVFromLuminance( float _luminance )
    {
        const float K = 12.5f; // typical calibration constant for DSLRs and spot meters
        return log2f( _luminance * 100.0f / K );
    }
}
