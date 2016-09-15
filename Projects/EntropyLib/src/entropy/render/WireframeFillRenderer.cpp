#include "WireframeFillRenderer.h"
#include "ofGraphics.h"
#include "entropy/Helpers.h"
#include <regex>

namespace entropy
{
	namespace render
	{
		std::string replace(std::string src, std::string find, std::string replacement){
			auto pos = src.find(find);
			auto len = find.length();
			return src.replace(pos, len, replacement);
		}

		using namespace glm;
		float smoothstep(float edge0, float edge1, float x){
			float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
			return t * t * (3.0 - 2.0 * t);
		}

		float smootherstep(float edge0, float edge1, float x){
			// Scale, and clamp x to 0..1 range
			x = clamp((x - edge0)/(edge1 - edge0), 0.0f, 1.0f);
			// Evaluate polynomial
			return x*x*x*(x*(x*6 - 15) + 10);
		}


		float fog(float dist, float startDist, float minDist, float maxDist, float power) {
			return pow(smootherstep(startDist, minDist, dist), 1./power) * pow(1-smootherstep(minDist, maxDist, dist), 1./power);
		}

		void WireframeFillRenderer::setup(float sceneSize){
			this->sceneSize = sceneSize;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.vert";
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.frag";
			shaderSettings.bindDefaults = true;
			shaderSettings.intDefines["FOG_ENABLED"] = fogEnabled;
			shaderSettings.intDefines["SHADE_NORMALS"] = shadeNormals;

			shaderSettings.intDefines["SPHERICAL_CLIP"] = 0;

			shaderSettings.intDefines["WIREFRAME"] = 0;
			shaderFill.setup(shaderSettings);

			shaderSettings.intDefines["WIREFRAME"] = 1;
			shaderWireframe.setup(shaderSettings);
			std::string postFragmentSource = R"(
				float fog(float dist, float startDist, float minDist, float maxDist, float power) {
					return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
				}

				vec4 postFragment(vec4 localColor){
					vec2 texcoord = vec2(gl_FragCoord.x / screenW, gl_FragCoord.y / screenH);
					texcoord.y = 1 - texcoord.y;
					float minDepth = texture(minDepthMask, texcoord).r;
					float maxDepth = texture(maxDepthMask, texcoord).r;
					if(wobblyClip<1 || (gl_FragCoord.z<minDepth && gl_FragCoord.z>maxDepth)){
						float distanceToCamera = length(v_eyePosition);
						localColor.a *= fog(distanceToCamera, fogStartDistance, fogMinDistance, fogMaxDistance, fogPower);
						 #if SPHERICAL_CLIP
							 float distanceToCenter = length(v_worldPosition);
							 float sphere = 1 - pow(smoothstep(fadeEdge0, fadeEdge1, distanceToCenter), fadePower);
							 localColor.a *= sphere;
						 #endif
						return localColor;
					}else{
						discard;
					}
				})";
			ofMaterial::Settings settings;
			settings.ambient = ofFloatColor::white;
			settings.diffuse = ofFloatColor::white;
			settings.specular = ofFloatColor::white;
			settings.emissive = ofFloatColor::black;
			settings.postFragment = postFragmentSource;
			settings.customUniforms = R"(
				uniform float fogStartDistance;
				uniform float fogMinDistance;
				uniform float fogMaxDistance;
				uniform float fogPower;
				uniform sampler2D minDepthMask;
				uniform sampler2D maxDepthMask;
				uniform float screenW;
				uniform float screenH;
				uniform float wobblyClip;
				uniform float fadeEdge0;
				uniform float fadeEdge1;
				uniform float fadePower;
			)";
			material.setup(settings);

			settings.postFragment = "#define SPHERICAL_CLIP 1\n" + settings.postFragment;
			materialSphericalClip.setup(settings);

			listeners.push_back((fogEnabled.newListener([&](bool & enabled){
				shaderSettings.intDefines["FOG_ENABLED"] = enabled;
				shaderSettings.intDefines["SPHERICAL_CLIP"] = 0;

				shaderSettings.intDefines["WIREFRAME"] = 0;
				shaderFill.setup(shaderSettings);

				shaderSettings.intDefines["WIREFRAME"] = 1;
				shaderWireframe.setup(shaderSettings);

				shaderSettings.intDefines["SPHERICAL_CLIP"] = 1;

				shaderSettings.intDefines["WIREFRAME"] = 0;
				shaderFillSphere.setup(shaderSettings);

				shaderSettings.intDefines["WIREFRAME"] = 1;
				shaderWireframeSphere.setup(shaderSettings);
			})));
		}

		void WireframeFillRenderer::resize(float width, float height){
			this->blobMask.setup(width, height, 0.25f * sceneSize);
		}

		void WireframeFillRenderer::drawDebug(){
			this->blobMask.getMaxDepthMask().draw(0,0);
		}

		void WireframeFillRenderer::draw(const ofVbo & geometry, size_t offset, size_t numVertices, ofCamera & camera) const{
			if(wobblyClip){
				this->blobMask.updateWith(camera);
			}

			ofDisableDepthTest();
			ofShader shaderFill;
			ofShader shaderWireframe;
			if(sphericalClip && clip){
				shaderFill = this->shaderFillSphere;
				shaderWireframe = this->shaderWireframeSphere;
			}else{
				shaderFill = this->shaderFill;
				shaderWireframe = this->shaderWireframe;
			}
			if (fill) {
				shaderFill.begin();
				shaderFill.setUniform1f("fogStartDistance", fogStartDistance);
				shaderFill.setUniform1f("fogMinDistance", fogMinDistance);
				shaderFill.setUniform1f("fogMaxDistance", fogMaxDistance);
				shaderFill.setUniform1f("fogPower", fogPower);
				shaderFill.setUniform1f("fadeEdge0", fadeEdge0);
				shaderFill.setUniform1f("fadeEdge1", fadeEdge1);
				shaderFill.setUniform1f("fadePower", fadePower);
				shaderFill.setUniform1f("alpha", fillAlpha * alphaFactor);
				shaderFill.setUniform1f("wobblyClip", wobblyClip && clip);
				shaderFill.setUniform1f("screenW", ofGetViewportWidth());
				shaderFill.setUniform1f("screenH", ofGetViewportHeight());
				shaderFill.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
				shaderFill.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
				geometry.draw(GL_TRIANGLES, offset, numVertices);
				shaderFill.end();

				if(useLights){
					material.begin();
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					geometry.draw(GL_TRIANGLES, offset, numVertices);
					material.end();
				}
			}

			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				if(useLights){
					auto * material = (clip && sphericalClip) ? &this->materialSphericalClip : &this->material;
					material->begin();
					material->setCustomUniform1f("fogStartDistance", fogStartDistance);
					material->setCustomUniform1f("fogMinDistance", fogMinDistance);
					material->setCustomUniform1f("fogMaxDistance", fogMaxDistance);
					material->setCustomUniform1f("fogPower", fogPower);
					material->setCustomUniform1f("fadeEdge0", fadeEdge0);
					material->setCustomUniform1f("fadeEdge1", fadeEdge1);
					material->setCustomUniform1f("fadePower", fadePower);
					material->setCustomUniform1f("wobblyClip", wobblyClip && clip);
					material->setCustomUniform1f("screenW", ofGetViewportWidth());
					material->setCustomUniform1f("screenH", ofGetViewportHeight());
					material->setCustomUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					material->setCustomUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.draw(GL_TRIANGLES, offset, numVertices);
					material->end();
				}else{
					shaderWireframe.begin();
					shaderWireframe.setUniform1f("fogStartDistance", fogStartDistance);
					shaderWireframe.setUniform1f("fogMinDistance", fogMinDistance);
					shaderWireframe.setUniform1f("fogMaxDistance", fogMaxDistance);
					shaderWireframe.setUniform1f("fogPower", fogPower);
					shaderWireframe.setUniform1f("fadeEdge0", fadeEdge0);
					shaderWireframe.setUniform1f("fadeEdge1", fadeEdge1);
					shaderWireframe.setUniform1f("fadePower", fadePower);
					shaderWireframe.setUniform1f("alpha", wireframeAlpha * alphaFactor);
					shaderWireframe.setUniform1f("wobblyClip", wobblyClip && clip);
					shaderWireframe.setUniform1f("screenW", ofGetViewportWidth());
					shaderWireframe.setUniform1f("screenH", ofGetViewportHeight());
					shaderWireframe.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					shaderWireframe.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.draw(GL_TRIANGLES, offset, numVertices);
					shaderWireframe.end();
				}
			}
		}

		void WireframeFillRenderer::drawElements(const ofVbo & geometry, size_t offset, size_t numIndices, ofCamera & camera) const{
			if(wobblyClip){
				this->blobMask.updateWith(camera);
			}

			ofDisableDepthTest();
			ofShader shaderFill;
			ofShader shaderWireframe;
			if(sphericalClip && clip){
				shaderFill = this->shaderFillSphere;
				shaderWireframe = this->shaderWireframeSphere;
			}else{
				shaderFill = this->shaderFill;
				shaderWireframe = this->shaderWireframe;
			}
			if (fill) {
				shaderFill.begin();
				shaderFill.setUniform1f("fogStartDistance", fogStartDistance);
				shaderFill.setUniform1f("fogMinDistance", fogMinDistance);
				shaderFill.setUniform1f("fogMaxDistance", fogMaxDistance);
				shaderFill.setUniform1f("fogPower", fogPower);
				shaderFill.setUniform1f("fadeEdge0", fadeEdge0);
				shaderFill.setUniform1f("fadeEdge1", fadeEdge1);
				shaderFill.setUniform1f("fadePower", fadePower);
				shaderFill.setUniform1f("alpha", fillAlpha * alphaFactor);
				shaderFill.setUniform1f("wobblyClip", wobblyClip && clip);
				shaderFill.setUniform1f("screenW", ofGetViewportWidth());
				shaderFill.setUniform1f("screenH", ofGetViewportHeight());
				shaderFill.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
				shaderFill.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				geometry.drawElements(GL_TRIANGLES, numIndices, offset);
				shaderFill.end();
			}

			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				if(useLights){
					auto * material = (clip && sphericalClip) ? &this->materialSphericalClip : &this->material;
					material->begin();
					material->setCustomUniform1f("fogStartDistance", fogStartDistance);
					material->setCustomUniform1f("fogMinDistance", fogMinDistance);
					material->setCustomUniform1f("fogMaxDistance", fogMaxDistance);
					material->setCustomUniform1f("fogPower", fogPower);
					material->setCustomUniform1f("fadeEdge0", fadeEdge0);
					material->setCustomUniform1f("fadeEdge1", fadeEdge1);
					material->setCustomUniform1f("fadePower", fadePower);
					material->setCustomUniform1f("wobblyClip", wobblyClip && clip);
					material->setCustomUniform1f("screenW", ofGetViewportWidth());
					material->setCustomUniform1f("screenH", ofGetViewportHeight());
					material->setCustomUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					material->setCustomUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.drawElements(GL_TRIANGLES, numIndices, offset);
					material->end();
				}else{
					shaderWireframe.begin();
					shaderWireframe.setUniform1f("fogStartDistance", fogStartDistance);
					shaderWireframe.setUniform1f("fogMinDistance", fogMinDistance);
					shaderWireframe.setUniform1f("fogMaxDistance", fogMaxDistance);
					shaderWireframe.setUniform1f("fogPower", fogPower);
					shaderWireframe.setUniform1f("fadeEdge0", fadeEdge0);
					shaderWireframe.setUniform1f("fadeEdge1", fadeEdge1);
					shaderWireframe.setUniform1f("fadePower", fadePower);
					shaderWireframe.setUniform1f("alpha", wireframeAlpha * alphaFactor);
					shaderWireframe.setUniform1f("wobblyClip", wobblyClip && clip);
					shaderWireframe.setUniform1f("screenW", ofGetViewportWidth());
					shaderWireframe.setUniform1f("screenH", ofGetViewportHeight());
					shaderWireframe.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					shaderWireframe.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.drawElements(GL_TRIANGLES, numIndices, offset);
					shaderWireframe.end();
				}
			}
		}

		std::vector<float> WireframeFillRenderer::getFogFunctionPlot(size_t numberOfPoints) const {
			std::vector<float> plot(numberOfPoints);
			for (size_t i = 0; i < numberOfPoints; i++) {
				float distanceToCamera = i/float(numberOfPoints) * 10.;
				plot[i] = fog(distanceToCamera, fogStartDistance, fogMinDistance, fogMaxDistance, fogPower);
			}
			return plot;
		}
	}
}
