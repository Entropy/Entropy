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
			void draw(ofxTexture3d & isoLevels);
			std::vector<float> getFogFunctionPlot(size_t numberOfPoints) const;

			ofParameter<int> resolution{ "Resolution", 1, 1, 512 };
			ofParameter<float> isoLevel{ "IsoLevel", 0.3f, 0.0f, 1.0f };
			ofParameter<bool> wireframe{ "Wireframe", true };
			ofParameter<bool> fill{ "Fill", true };
			ofParameter<bool> shadeNormals{ "Shade Normals", true };
			ofParameter<float> fogMaxDistance{ "Fog max dist.", 1.5f, 0.2f, 10.f };
			ofParameter<float> fogMinDistance{ "Fog min dist.", 0.1f, 0.0f, 5.f };
			ofParameter<float> fogPower{ "Fog power", 1.f, 0.001f, 10.f };
			ofParameter<bool> fogEnabled{ "Fog enabled", true };
			ofParameter<float> wireframeAlpha{ "Wireframe alpha", 0.25f, 0.f, 1.f };
			ofParameter<float> fillAlpha{ "Fill alpha", 0.5f, 0.f, 1.f };

			ofParameterGroup parameters{
				"Marching Cubes",
				resolution,
				wireframe,
				fill,
				shadeNormals,
				fogMaxDistance,
				fogMinDistance,
				fogPower,
				fogEnabled,
				wireframeAlpha,
				fillAlpha,
			};

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
