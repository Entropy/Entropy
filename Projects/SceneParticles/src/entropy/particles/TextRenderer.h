#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include "ofConstants.h"
#include "WireframeFillRenderer.h"
#include "ofEvents.h"
#include "ofParameter.h"
#include "ofTrueTypeFont.h"
#include "ParticleSystem.h"
#include "Photons.h"


class TextRenderer
{
public:
	void setup(float	 worldSize);
	void update(nm::ParticleSystem & particles, nm::Environment & environment);
	void draw(nm::ParticleSystem & particles,
			  std::vector<nm::Photon> & photons,
			  nm::Environment & environment,
			  std::pair<nm::Particle*, nm::Particle*> lookAt,
			  entropy::render::WireframeFillRenderer & renderer,
			  ofCamera & cam);

	ofParameter<float> maxDistance{"max distance", 1, 0, 4};
	ofParameter<float> relDistance{"relation distance", 0.05f, 0, 2};
	ofParameter<float> fulltextDistance{"fulltext distance", 0.05f, 0, 2};
	ofParameter<float> lightStrenght{"light strenght", 100.f, 1.f, 2000.f};
	ofParameter<float> ambient{"ambient", .1f, 0.f, 1.f};
	ofParameter<float> photonsStrenght{"photons strenght", 0.5f, 0.1f, 300.f};

	ofParameterGroup parameters{
		"Text Renderer",
		maxDistance,
		relDistance,
		fulltextDistance,
		lightStrenght,
		ambient,
		photonsStrenght,
	};

private:
	std::vector<ofEventListener> listeners;
	std::vector<ofTrueTypeFont> fonts;
	ofShader billboardShaderText;
	ofShader billboardShaderPath;
	std::vector<std::unordered_map<std::string, ofMesh>> particleTexts;
	std::vector<std::unordered_map<std::string, ofPath>> particlePaths;
	float worldSize;
};

#endif // TEXTRENDERER_H
