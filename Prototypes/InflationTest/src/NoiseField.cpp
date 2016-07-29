//
//  NoiseField.cpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#include "NoiseField.h"
#include "ofNoise.h"
#include "Constants.h"
#include <regex>

namespace entropy
{
    //--------------------------------------------------------------
    void NoiseField::compileComputeShader(){
        ofFile computeFile("shaders/compute_noise4d.glsl");
        ofBuffer computeBuff(computeFile);
        std::string computeSource = computeBuff.getText();

        std::regex re_clip("const[ \t]+bool[ \t]+sphericalClip[ \t]*=.*;");
        computeSource = std::regex_replace(computeSource, re_clip, std::string("const bool sphericalClip = ") + (sphericalClip ? "true;" : "false;"));

        std::regex re_fill("const[ \t]+bool[ \t]+fillEdges[ \t]*=.*;");
        computeSource = std::regex_replace(computeSource, re_fill, std::string("const bool fillEdges = ") + (fillEdges ? "true;" : "false;"));

        noiseComputeShader.setupShaderFromSource(GL_COMPUTE_SHADER, computeSource);
        noiseComputeShader.linkProgram();
    }

    //--------------------------------------------------------------
    void NoiseField::allocateVolumeTexture(){
        volumeShader.load("shaders/volumetrics_vertex.glsl", "shaders/volumetrics_frag.glsl");
        volumeTex.allocate(resolution,resolution,resolution,GL_RGBA16F);
        auto clearData = std::vector<ofFloatColor>(resolution*resolution*resolution, ofFloatColor::black);
        volumeTex.loadData(reinterpret_cast<float*>(clearData.data()),resolution,resolution,resolution,0,0,0,GL_RGBA);
        //volumeTex.setMinMagFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
        volumetrics.setup(&volumeTex, {1,1,1}, volumeShader);
    }

    //--------------------------------------------------------------
    void NoiseField::setup(ofParameter<int> & resolution){
        this->resolution.makeReferenceTo(resolution);

        resolutionListener = resolution.newListener([&](int & resolution){
            allocateVolumeTexture();
        });

        sphericalClipListener = sphericalClip.newListener([&](bool & clip){
            if(fillEdges){
                fillEdges = false;
            }else{
                compileComputeShader();
            }
        });

        fillEdgesListener = fillEdges.newListener([&](bool & fill){
            if(!fill || !sphericalClip){
                compileComputeShader();
            }else{
                fillEdges = false;
            }
        });

        allocateVolumeTexture();
        compileComputeShader();
	}

    //--------------------------------------------------------------
    void NoiseField::update(bool inflation)
    {
        auto i = 0;
        for(auto & octave: octaves){
            if(octave.advanceTime){
                octave.now += noiseSpeed * ofGetLastFrameTime();
            }
        }

        if(inflation){
            octaves[3].frequency *= 0.99;
            if(octaves[3].frequency<10){
                octaves[3].advanceTime = false;
            }
            if(octaves[3].frequency<2){
                octaves[3].advanceTime = true;
                octaves[3].frequency = 120;
            }
        }

#if USE_GPU_NOISE
		noiseComputeShader.begin();
		volumeTex.bindAsImage(0,GL_WRITE_ONLY,0,true,0);
        for(size_t i=0;i<octaves.size();++i){
            auto octave = "octaves[" + ofToString(i) + "].";
            noiseComputeShader.setUniform1f(octave + "now", octaves[i].now);
            noiseComputeShader.setUniform1f(octave + "frequency", octaves[i].frequency);
            noiseComputeShader.setUniform1f(octave + "amplitude", octaves[i].amplitude);
            noiseComputeShader.setUniform1f(octave + "enabled", octaves[i].enabled ? 1.0f : 0.0f);
        }
        noiseComputeShader.setUniform1f("resolution", resolution);
        noiseComputeShader.setUniform1f("inputMultiplier" , inputMultiplier);
		noiseComputeShader.setUniform1f("normalizationFactor", normalizationFactor);
        noiseComputeShader.setUniform1f("fade", (float)fadeAt);
        noiseComputeShader.dispatchCompute(resolution/8, resolution/8, resolution/8);
		noiseComputeShader.end();
		glBindImageTexture(0,0,0,0,0,GL_READ_WRITE,GL_R16F);
		glMemoryBarrier( GL_SHADER_IMAGE_ACCESS_BARRIER_BIT );
		//volumeTex.generateMipmaps();
#endif
    }

    //--------------------------------------------------------------
    void NoiseField::numScalesChanged(int& numScales)
	{
    }

    //--------------------------------------------------------------
    float NoiseField::getValue(int x, int y, int z)
    {
		auto total = 0.;
		auto maxValue = 0.;
		auto centeredPos = glm::vec3(x - resolution/2., y - resolution/2., z - resolution/2.);
        auto pos = centeredPos * (float)inputMultiplier;
        // fade out from the fade limit until 1
        /*float sphere = 1;
        if(sphericalClip){
            glm::vec3 normPos = centeredPos / (resolution/2.);
            float distance = glm::dot(normPos, normPos);
            float edge = 1.0 - fadeAt;
            float curveStart = int(distance+edge);
            float fadeOut = glm::clamp((1.f - (distance - fadeAt) / edge), 0.f, 1.f);
            sphere = 1.0 - curveStart + curveStart * fadeOut * fadeOut;
        }*/
        for(auto & octave: octaves){
            //float radius = octave.radius * resolution;
            if(octave.enabled){// &&  radius * radius > distance2){
                auto freqD = octave.frequency / 60.f;
                auto amplitude = octave.amplitude;// * (1. - ofClamp(normDistance, 0, 1));
                total +=  (_slang_library_noise4(pos.x*freqD, pos.y*freqD, pos.z*freqD, octave.now*freqD)*0.5f + 0.5f) * amplitude;
                maxValue += amplitude;
            }
        }
        return total/(maxValue*normalizationFactor);
    }

    //--------------------------------------------------------------
	void NoiseField::draw(float threshold){
		volumetrics.setRenderSettings(1, 1, 1, threshold);
		volumetrics.drawVolume(0, 0, 0, 256, 0);
	}
}
