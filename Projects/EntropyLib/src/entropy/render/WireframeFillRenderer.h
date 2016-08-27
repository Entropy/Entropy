#ifndef WIREFRAMEFILLRENDERER_H
#define WIREFRAMEFILLRENDERER_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofMaterial.h"

namespace entropy
{
    namespace render
    {
        class WireframeFillRenderer
        {
        public:
            void setup();
            void draw(const ofVbo & geometry, size_t offset, size_t numVertices) const;
            void drawElements(const ofVbo & geometry, size_t offset, size_t numIndices) const;
            std::vector<float> getFogFunctionPlot(size_t numberOfPoints) const;


            ofParameter<bool> wireframe{ "Wireframe", true };
            ofParameter<bool> fill{ "Fill", true };
            ofParameter<float> fogMaxDistance{ "Fog max dist.", 1.5f, 0.2f, 10.f };
            ofParameter<float> fogMinDistance{ "Fog min dist.", 0.1f, 0.0f, 5.f };
			ofParameter<float> fogStartDistance{ "Fog start dist.", 0.1f, 0.0f, 5.f };
            ofParameter<float> fogPower{ "Fog power", 1.f, 0.001f, 10.f };
            ofParameter<bool> fogEnabled{ "Fog enabled", true };
			ofParameter<float> fadeEdge0{ "Fade From", 0.8f, 0.0f, 1.0f };
			ofParameter<float> fadeEdge1{ "Fade To", 1.0f, 0.0f, 1.0f };
			ofParameter<float> fadePower{ "Fade power", 2.f, 1.0f, 10.0f };
			ofParameter<bool> sphericalClip{ "Spherical Clip", false };
            ofParameter<float> wireframeAlpha{ "Wireframe alpha", 0.25f, 0.f, 1.f };
            ofParameter<float> fillAlpha{ "Fill alpha", 0.5f, 0.f, 1.f };
            ofParameter<bool> shadeNormals{ "Shade Normals", false };
            ofParameter<bool> useLights{ "Use lights", false };

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
                wireframeAlpha,
                fillAlpha,
                useLights,
            };

		private:
            void setMaterial();
            ofShader shaderFill, shaderWireframe;
			ofShader shaderFillSphere, shaderWireframeSphere;
			ofShader::Settings shaderSettings;
            std::vector<ofEventListener> listeners;
            ofMaterial material;
        };
    }
}
#endif // WIREFRAMEFILLRENDERER_H
