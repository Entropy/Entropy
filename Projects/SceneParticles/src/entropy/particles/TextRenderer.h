#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include "ofConstants.h"
#include "WireframeFillRenderer.h"
#include "ofEvents.h"
#include "ofParameter.h"
#include "ofTrueTypeFont.h"
#include "ParticleSystem.h"

enum State{
	BARYOGENESIS,
	STANDARD_MODEL,
	NUCLEOSYNTHESIS,
};

class TextRenderer
{
public:
	void setup(float worldSize);
	void update(nm::ParticleSystem & particles, State state);
	void draw(nm::ParticleSystem & particles,
			  nm::Environment & environment,
			  State state,
			  entropy::render::WireframeFillRenderer & renderer,
			  ofCamera & cam);

	ofParameter<float> maxDistance{"max distance", 1, 0, 4};
	ofParameter<float> relDistance{"relation distance", 0.05f, 0, 2};
	ofParameter<float> fulltextDistance{"fulltext distance", 0.05f, 0, 2};

	ofParameterGroup parameters{
		"parameters",
		maxDistance,
		relDistance,
		fulltextDistance,
	};

private:
	std::vector<std::pair<size_t,std::vector<nm::Particle*>>> relations;
	std::vector<ofEventListener> listeners;
	std::vector<ofTrueTypeFont> fonts;
	ofShader billboardShader;
	std::vector<std::unordered_map<std::string, ofMesh>> particleTexts;
	float worldSize;
};

#endif // TEXTRENDERER_H
