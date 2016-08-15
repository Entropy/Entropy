#ifndef GPUMARCHINGCUBES_H
#define GPUMARCHINGCUBES_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofxTexture3d.h"

namespace entropy
{
	namespace inflation
	{
		class GPUMarchingCubes
		{
		public:
			void setup();
			void draw(ofxTexture3d & isoLevels, float threshold);

			ofParameter<int> resolution{ "Resolution", 1, 1, 512 };
			ofParameter<bool> wireframe{ "Wireframe", true };
			ofParameter<bool> fill{ "Fill", true };
			ofParameter<bool> shadeNormals{ "Shade Normals", true };
			ofParameter<float> fogMaxDistance{ "Fog max dist.", 1.5f, 0.2f, 5.f };

		private:
			void compileShader();
			ofVbo vbo;
			ofShader shaderWireframe, shaderFill;
			ofTexture triTableTex;
			ofEventListener resolutionListener, wireFrameListener, shadeNormalsListener;
		};
	}
}

#endif // GPUMARCHINGCUBES_H
