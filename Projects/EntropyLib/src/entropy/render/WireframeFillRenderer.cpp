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

		float fog(float dist, float startDist, float minDist, float maxDist, float power) {
			return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
		}

        void WireframeFillRenderer::setMaterial(){
            std::string postFragmentSource = R"(
				float fog(float dist, float startDist, float minDist, float maxDist, float power) {
					return pow(smoothstep(startDist, minDist, dist), 1./power) * pow(1-smoothstep(minDist, maxDist, dist), 1./power);
				}

				vec4 postFragment(vec4 localColor){
					float distanceToCamera = length(v_eyePosition);
					localColor.a *= fog(distanceToCamera, %fogStartDistance%, %fogMinDistance%, %fogMaxDistance%, %fogPower%);
					return localColor;
				})";
			postFragmentSource = replace(postFragmentSource, "%fogStartDistance%", std::to_string(fogStartDistance));
            postFragmentSource = replace(postFragmentSource, "%fogMinDistance%", std::to_string(fogMinDistance));
            postFragmentSource = replace(postFragmentSource, "%fogMaxDistance%", std::to_string(fogMaxDistance));
            postFragmentSource = replace(postFragmentSource, "%fogPower%", std::to_string(fogPower));
            material.setColors(ofFloatColor::white, ofFloatColor::white, ofFloatColor::white, ofFloatColor::black);
            material.setPostFragment(postFragmentSource);
        }

		void WireframeFillRenderer::setup(){
			cout << "compiling " << GetShadersPath(Module::Renderers) / "wireframeFillRender.vert" << endl;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.vert";
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.frag";
			shaderSettings.bindDefaults = true;
			shaderSettings.intDefines["FOG_ENABLED"] = fogEnabled;
			shaderSettings.intDefines["SHADE_NORMALS"] = shadeNormals;

			shaderSettings.intDefines["WIREFRAME"] = 0;
			shaderFill.setup(shaderSettings);

			shaderSettings.intDefines["WIREFRAME"] = 1;
			shaderWireframe.setup(shaderSettings);

            setMaterial();

            listeners.push_back(fogMinDistance.newListener([&](float &){
                setMaterial();
            }));
            listeners.push_back(fogMaxDistance.newListener([&](float &){
                setMaterial();
            }));
            listeners.push_back(fogPower.newListener([&](float &){
                setMaterial();
            }));
			listeners.push_back((fogEnabled.newListener([&](bool & enabled){
				cout << "recompiling" << endl;
				shaderSettings.intDefines["FOG_ENABLED"] = enabled;

				shaderSettings.intDefines["WIREFRAME"] = 0;
				shaderFill.setup(shaderSettings);

				shaderSettings.intDefines["WIREFRAME"] = 1;
				shaderWireframe.setup(shaderSettings);
			})));
        }

        void WireframeFillRenderer::draw(const ofVbo & geometry, size_t offset, size_t numVertices) const{
            ofDisableDepthTest();
            if (fill) {
                shaderFill.begin();
				shaderFill.setUniform1f("fogStartDistance", fogStartDistance);
                shaderFill.setUniform1f("fogMinDistance", fogMinDistance);
                shaderFill.setUniform1f("fogMaxDistance", fogMaxDistance);
                shaderFill.setUniform1f("fogPower", fogPower);
                shaderFill.setUniform1f("alpha", fillAlpha);
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
                    material.begin();
                    geometry.draw(GL_TRIANGLES, offset, numVertices);
                    material.end();
                }else{
                    shaderWireframe.begin();
					shaderWireframe.setUniform1f("fogStartDistance", fogStartDistance);
                    shaderWireframe.setUniform1f("fogMinDistance", fogMinDistance);
                    shaderWireframe.setUniform1f("fogMaxDistance", fogMaxDistance);
                    shaderWireframe.setUniform1f("fogPower", fogPower);
                    shaderWireframe.setUniform1f("alpha", wireframeAlpha);
                    geometry.draw(GL_TRIANGLES, offset, numVertices);
                    shaderWireframe.end();
                }
            }
        }

        void WireframeFillRenderer::drawElements(const ofVbo & geometry, size_t offset, size_t numIndices) const{
            ofDisableDepthTest();
            if (fill) {
                shaderFill.begin();
				shaderFill.setUniform1f("fogStartDistance", fogStartDistance);
                shaderFill.setUniform1f("fogMinDistance", fogMinDistance);
                shaderFill.setUniform1f("fogMaxDistance", fogMaxDistance);
                shaderFill.setUniform1f("fogPower", fogPower);
                shaderFill.setUniform1f("alpha", fillAlpha);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                geometry.drawElements(GL_TRIANGLES, numIndices, offset);
                shaderFill.end();
            }

            if (wireframe) {
                shaderWireframe.begin();
				shaderWireframe.setUniform1f("fogStartDistance", fogStartDistance);
                shaderWireframe.setUniform1f("fogMinDistance", fogMinDistance);
                shaderWireframe.setUniform1f("fogMaxDistance", fogMaxDistance);
                shaderWireframe.setUniform1f("fogPower", fogPower);
                shaderWireframe.setUniform1f("alpha", wireframeAlpha);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                geometry.drawElements(GL_TRIANGLES, numIndices, offset);
                shaderWireframe.end();
            }
            material.end();
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
