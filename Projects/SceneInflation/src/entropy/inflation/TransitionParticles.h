#ifndef TRANSITIONPARTICLES_H
#define TRANSITIONPARTICLES_H

#include "ofBufferObject.h"
#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameterGroup.h"
#include "ofParameter.h"
#include "ofxTexture3d.h"

class TransitionParticles
{
public:
	void setup();

	void update(const ofBufferObject & blobs, const ofxTexture3d & noiseField, float now);
	void setTotalVertices(int totalVertices);
	int getNumVertices() const;
	const ofVbo & getVbo() const;

	ofParameter<int> every{"particle every x vertices", 100, 1, 10000};
	ofParameter<float> scale{"scale", 1, 0.1f, 5};
	ofParameter<float> repulsion{"repulsion", 0.01, 0.001f, 1.f};
	ofParameter<float> attraction{"noise field attraction", 10.f, 0.f, 10.f};
	ofParameter<float> speed{"speed", 1.f/100.f, 1.f/1000.f, 1.f/10.f};
	ofParameter<ofFloatColor> color{"color", ofFloatColor::white};
	ofParameterGroup parameters{
		"transition particles",
		every,
		scale,
		color,
		repulsion,
		attraction,
		speed,
	};

private:
	ofVbo model;
	ofBufferObject feedbackBuffer;
	ofVbo positions;
	ofShader shader;
	ofShader computeShader;
	int totalVertices;
	GLuint numVerticesQuery;
	GLuint numPrimitives;
	size_t modelNumVertices;
};

#endif // TRANSITIONPARTICLES_H
