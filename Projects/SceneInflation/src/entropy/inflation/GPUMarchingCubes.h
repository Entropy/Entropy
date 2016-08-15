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
			std::vector<float> getFogFunctionPlot(size_t numberOfPoints) const;

			ofParameter<int> resolution{ "Resolution", 1, 1, 512 };
			ofParameter<bool> wireframe{ "Wireframe", true };
			ofParameter<bool> fill{ "Fill", true };
			ofParameter<bool> shadeNormals{ "Shade Normals", true };
			ofParameter<float> fogMaxDistance{ "Fog max dist.", 1.5f, 0.2f, 10.f };
			ofParameter<float> fogMinDistance{ "Fog min dist.", 0.1f, 0.0f, 5.f };
			ofParameter<float> fogPower{ "Fog power", 1.f, 0.001f, 10.f };
			ofParameter<bool> fogEnabled{ "Fog enabled", true };

		private:
			void compileShader();
			ofVbo vbo;
			ofShader shaderWireframe, shaderFill;
			ofTexture triTableTex;
			ofEventListener resolutionListener, wireFrameListener, shadeNormalsListener, fogEnabledListener;
		};
	}
}

#endif // GPUMARCHINGCUBES_H
