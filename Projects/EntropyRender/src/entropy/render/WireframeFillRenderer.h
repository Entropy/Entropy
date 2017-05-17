#ifndef WIREFRAMEFILLRENDERER_H
#define WIREFRAMEFILLRENDERER_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofParameterGroup.h"
#include "ofMaterial.h"
#include "BlobMask.h"
#include "Helpers.h"

namespace entropy
{
	namespace render
	{
		class WireframeFillRenderer
		{
		public:


			struct Parameters : public ofParameterGroup{
				ofParameter<bool> wireframe{ "Wireframe", true };
				ofParameter<bool> fill{ "Fill", true };
				ofParameter<float> fogMaxDistance{ "Fog Max Distance", 1.5f, 0.2f, 3.f, ofParameterScale::Logarithmic };
				ofParameter<float> fogMinDistance{ "Fog Min Distance", 0.1f, 0.0f, 1.f, ofParameterScale::Logarithmic };
				ofParameter<float> fogStartDistance{ "Fog Start Distance", 0.1f, 0.0f, 1.f, ofParameterScale::Logarithmic };
				ofParameter<float> fogPower{ "Fog Power", 1.f, 0.001f, 3.f, ofParameterScale::Logarithmic };
				ofParameter<bool> fogEnabled{ "Fog Enabled", true };
				ofParameter<float> fadeEdge0{ "Fade From", 0.8f, 0.0f, 1.0f, ofParameterScale::Logarithmic };
				ofParameter<float> fadeEdge1{ "Fade To", 1.0f, 0.0f, 1.0f, ofParameterScale::Logarithmic };
				ofParameter<float> fadePower{ "Fade Power", 2.f, 1.0f, 10.0f, ofParameterScale::Logarithmic };
				ofParameter<bool> clip{ "Clip", false };
				ofParameter<bool> sphericalClip{ "Spherical Clip", false };
				ofParameter<bool> wobblyClip{ "Wobbly Clip", false };
				ofParameter<float> wireframeAlpha{ "Wireframe Alpha", 0.25f, 0.f, 1.f, ofParameterScale::Logarithmic };
				ofParameter<float> fillAlpha{ "Fill Alpha", 0.5f, 0.f, 1.f, ofParameterScale::Logarithmic };
				ofParameter<float> alphaFactor{"Alpha factor", 1.0f, 0.f, 1.f, ofParameterScale::Logarithmic};
				ofParameter<bool> shadeNormals{ "Shade Normals", false };
				ofParameter<bool> useLights{ "Use Lights", false };
				ofParameter<bool> enableDOF{ "enable DOF", false };
				ofParameter<int> dofSamples{ "DOF samples", 1, 1, 512 };
				ofParameter<float> dofAperture{ "DOF aperture", 0.0005, 0.0, 0.05};
				ofParameter<float> dofDistance{ "DOF distance", 1, 0.01, 2};
				ofParameter<int> bokehsides{ "Bokeh sides", 6, 4, 200 };

				PARAM_DECLARE(
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
					clip,
					sphericalClip,
					wobblyClip,
					wireframeAlpha,
					fillAlpha,
					alphaFactor,
					useLights,
					enableDOF,
					dofSamples,
					dofAperture,
					dofDistance,
					bokehsides
				);
			} parameters;

			struct DrawSettings{
				size_t offset;
				size_t numVertices;
				GLenum mode;
				ofCamera * camera;
				glm::mat4 model;
				Parameters parameters;
			};


			void setup(float sceneSize);
			void resize(float width, float height);
			void draw(const ofVbo & geometry, size_t offset, size_t numVertices, GLenum mode, ofCamera & camera, const glm::mat4 & model = glm::mat4()) const;
			void draw(const ofVbo & geometry, DrawSettings settings) const;
			void drawElements(const ofVbo & geometry, size_t offset, size_t numIndices, GLenum mode, ofCamera & camera) const;
			void drawWithDOF(ofCamera & camera, std::function<void(float accumValue, glm::mat4 projection, glm::mat4 modelview)> drawFunc) const;
			void drawDebug();
			std::vector<float> getFogFunctionPlot(size_t numberOfPoints) const;
			const ofMesh & getBokehShape() const;
		private:
			ofMesh bokehshape;
			ofShader shaderFill, shaderWireframe;
			ofShader shaderFillSphere, shaderWireframeSphere;
			ofShader::Settings shaderSettings;
			std::vector<ofEventListener> listeners;
			mutable BlobMask blobMask;
			mutable ofMaterial material;
			mutable ofMaterial materialSphericalClip;
			float sceneSize;
			mutable std::vector<glm::mat4> modelview;
			mutable std::vector<glm::mat4> mvp;
		};
	}
}
#endif // WIREFRAMEFILLRENDERER_H
