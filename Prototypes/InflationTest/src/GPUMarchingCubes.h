#ifndef GPUMARCHINGCUBES_H
#define GPUMARCHINGCUBES_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofxTexture3d.h"

class GPUMarchingCubes
{
	public:
		void setup();
		void draw(ofxTexture3d & isoLevels, float threshold);

		ofParameter<int> resolution{"resolution", 1, 1, 512};
        ofParameter<bool> wireframe{"wireframe", true};

	private:
		ofVbo vbo;
		ofShader shader;
		ofTexture triTableTex;
		ofEventListener resolutionListener;
};

#endif // GPUMARCHINGCUBES_H
