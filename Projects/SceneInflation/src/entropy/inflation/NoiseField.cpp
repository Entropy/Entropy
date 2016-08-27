//
//  NoiseField.cpp
//  InflationTest
//
//  Created by Elias Zananiri on 2016-01-14.
//
//

#include "NoiseField.h"
#include "ofNoise.h"
#include <regex>

namespace entropy
{
	namespace inflation
	{
		//--------------------------------------------------------------
		NoiseField::NoiseField() {
			octaves.emplace_back(0, 20, 1.f);
			octaves.emplace_back(1, 40, 0.5f);
			octaves.emplace_back(2, 80, 0.25f);
			octaves.emplace_back(3, 160, 0.125f);

			for (auto & octave : octaves) {
				parameters.add(octave.parameters);
			}
        }

		//--------------------------------------------------------------
		void NoiseField::allocateVolumeTexture() {
			volumeShader.load("shaders/volumetrics_vertex.glsl", "shaders/volumetrics_frag.glsl");
			volumeTex.allocate(resolution, resolution, resolution, GL_RGBA16F);
			volumeValues.assign(resolution*resolution*resolution, ofFloatColor::black);
			volumeTex.loadData(reinterpret_cast<float*>(volumeValues.data()), resolution, resolution, resolution, 0, 0, 0, GL_RGBA);
			//volumeTex.setMinMagFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
			volumetrics.setup(&volumeTex, { 1,1,1 }, volumeShader);
		}

		//--------------------------------------------------------------
		void NoiseField::setup(ofParameter<int> & resolution) {
			this->resolution.makeReferenceTo(resolution);

			shaderSettings.shaderFiles[GL_COMPUTE_SHADER] = "shaders/compute_noise4d.glsl";
			shaderSettings.intDefines["FILL_EDGES"] = fillEdges;
            noiseComputeShader.setup(shaderSettings);
            allocateVolumeTexture();

			resolutionListener = resolution.newListener([&](int & resolution) {
				allocateVolumeTexture();
			});

			fillEdgesListener = fillEdges.newListener([&](bool & fill) {
				shaderSettings.intDefines["FILL_EDGES"] = fillEdges;
				noiseComputeShader.setup(shaderSettings);
			});

		}

		//--------------------------------------------------------------
		void NoiseField::update()
		{
			auto i = 0;
			for (auto & octave : octaves) {
				if (octave.advanceTime) {
					octave.now += noiseSpeed * ofGetLastFrameTime();
				}
			}

			noiseComputeShader.begin();
			volumeTex.bindAsImage(0, GL_WRITE_ONLY, 0, true, 0);
			for (size_t i = 0; i < octaves.size(); ++i) {
				auto octave = "octaves[" + ofToString(i) + "].";
				noiseComputeShader.setUniform1f(octave + "now", octaves[i].now);
				noiseComputeShader.setUniform1f(octave + "frequency", octaves[i].frequency);
				noiseComputeShader.setUniform1f(octave + "amplitude", octaves[i].amplitude);
				noiseComputeShader.setUniform1f(octave + "enabled", octaves[i].enabled ? 1.0f : 0.0f);
				noiseComputeShader.setUniform4f(octave + "color", octaves[i].color);
			}
			noiseComputeShader.setUniform1f("resolution", resolution);
			noiseComputeShader.setUniform1f("normalizationFactor", normalizationFactor);
			noiseComputeShader.dispatchCompute(resolution / 8, resolution / 8, resolution / 8);
			noiseComputeShader.end();
			glBindImageTexture(0, 0, 0, 0, 0, GL_READ_WRITE, GL_R16F);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			//volumeTex.generateMipmaps();
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
			auto pos = glm::vec3(x - resolution / 2., y - resolution / 2., z - resolution / 2.);
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
			for (auto & octave : octaves) {
				//float radius = octave.radius * resolution;
				if (octave.enabled) {// &&  radius * radius > distance2){
					auto freqD = octave.frequency;
					auto amplitude = octave.amplitude;// * (1. - ofClamp(normDistance, 0, 1));
					total += (_slang_library_noise4(pos.x*freqD, pos.y*freqD, pos.z*freqD, octave.now*freqD)*0.5f + 0.5f) * amplitude;
					maxValue += amplitude;
				}
			}
			return total / (maxValue*normalizationFactor);
		}

		//--------------------------------------------------------------
		void NoiseField::draw(float threshold) {
			volumetrics.setRenderSettings(1, 1, 1, threshold);
			volumetrics.drawVolume(0, 0, 0, 256, 0);
		}
	}
}
