#ifndef WIREFRAMEFILLRENDERER_H
#define WIREFRAMEFILLRENDERER_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofMaterial.h"
#include "BlobMask.h"

namespace entropy
{
	namespace render
	{
		class WireframeFillRenderer
		{
		public:
			void setup(float sceneSize);
			void resize(float width, float height);
			void draw(const ofVbo & geometry, size_t offset, size_t numVertices, ofCamera & camera) const;
			void drawElements(const ofVbo & geometry, size_t offset, size_t numIndices, ofCamera & camera) const;
			void drawDebug();
			std::vector<float> getFogFunctionPlot(size_t numberOfPoints) const;

			ofParameter<bool> wireframe{ "Wireframe", true };
			ofParameter<bool> fill{ "Fill", true };
			ofParameter<float> fogMaxDistance{ "Fog Max Distance", 1.5f, 0.2f, 10.f };
			ofParameter<float> fogMinDistance{ "Fog Min Distance", 0.1f, 0.0f, 5.f };
			ofParameter<float> fogStartDistance{ "Fog Start Distance", 0.1f, 0.0f, 5.f };
			ofParameter<float> fogPower{ "Fog Power", 1.f, 0.001f, 10.f };
			ofParameter<bool> fogEnabled{ "Fog Enabled", true };
			ofParameter<float> fadeEdge0{ "Fade From", 0.8f, 0.0f, 1.0f };
			ofParameter<float> fadeEdge1{ "Fade To", 1.0f, 0.0f, 1.0f };
			ofParameter<float> fadePower{ "Fade Power", 2.f, 1.0f, 10.0f };
			ofParameter<bool> clip{ "Clip", false };
			ofParameter<bool> sphericalClip{ "Spherical Clip", false };
			ofParameter<bool> wobblyClip{ "Wobbly Clip", false };
			ofParameter<float> wireframeAlpha{ "Wireframe Alpha", 0.25f, 0.f, 1.f };
			ofParameter<float> fillAlpha{ "Fill Alpha", 0.5f, 0.f, 1.f };
			ofParameter<float> alphaFactor{"Alpha factor", 1.0f, 0.f, 1.f};
			ofParameter<bool> shadeNormals{ "Shade Normals", false };
			ofParameter<bool> useLights{ "Use Lights", false };

			ofParameterGroup parameters{
				"Renderer",
				wireframe,
				fill,
				shadeNormals,
				fogStartDistance,
				fogMinDistance,
				fogMaxDistance,
				fogPower,
				fogEnabled,
				fadeEdge0,
				fadeEdge1,
				fadePower,
				sphericalClip,
				wobblyClip,
				wireframeAlpha,
				fillAlpha,
				useLights,
			};

		private:
			ofShader shaderFill, shaderWireframe;
			ofShader shaderFillSphere, shaderWireframeSphere;
			ofShader::Settings shaderSettings;
			std::vector<ofEventListener> listeners;
			mutable BlobMask blobMask;
			mutable ofMaterial material;
			mutable ofMaterial materialSphericalClip;
			float sceneSize;
		};
	}
}
#endif // WIREFRAMEFILLRENDERER_H
