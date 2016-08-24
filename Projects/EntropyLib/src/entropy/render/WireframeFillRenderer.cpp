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

        void WireframeFillRenderer::compileShader(){
            //fragment shader
            ofFile vertFile(GetShadersPath(Module::Renderers) / "wireframeFillRender.vert");
            ofBuffer vertBuff(vertFile);
            std::string vertSource = vertBuff.getText();

            std::regex re_fog_enabled("#define FOG_ENABLED [0-1]");
            vertSource = std::regex_replace(vertSource, re_fog_enabled, "#define FOG_ENABLED " + ofToString(fogEnabled));

            std::regex re_shade_normals("#define SHADE_NORMALS [0-1]");
            vertSource = std::regex_replace(vertSource, re_shade_normals, "#define SHADE_NORMALS " + ofToString(shadeNormals));

            std::regex re_wireframe("#define WIREFRAME [0-1]");

            vertSource = std::regex_replace(vertSource, re_wireframe, "#define WIREFRAME 0");
            shaderFill.setupShaderFromSource(GL_VERTEX_SHADER, vertSource);
            shaderFill.setupShaderFromFile(GL_FRAGMENT_SHADER, GetShadersPath(Module::Renderers) / "wireframeFillRender.frag");
            shaderFill.bindDefaults();
            shaderFill.linkProgram();

            vertSource = std::regex_replace(vertSource, re_wireframe, "#define WIREFRAME 1");
            shaderWireframe.setupShaderFromSource(GL_VERTEX_SHADER, vertSource);
            shaderWireframe.setupShaderFromFile(GL_FRAGMENT_SHADER, GetShadersPath(Module::Renderers) / "wireframeFillRender.frag");
            shaderWireframe.bindDefaults();
            shaderWireframe.linkProgram();
        }

        void WireframeFillRenderer::setMaterial(){
            std::string postFragmentSource = R"(
                 float fog(float dist, float minDist, float maxDist, float power) {
                      dist = pow(dist, power);
                      minDist = pow(minDist, power);
                      maxDist = pow(maxDist, power);
                      float invDistanceToCamera = 1 - clamp((dist - minDist) / maxDist, 0.f, 1.f);
                      if (dist > minDist) {
                          return invDistanceToCamera;
                      }
                      else {
                          return 1;
                      }
                  }
                  vec4 postFragment(vec4 localColor){
                     float distanceToCamera = length(v_eyePosition);
                     localColor.a *= fog(distanceToCamera, %fogMinDistance%, %fogMaxDistance%, %fogPower%);
                     return localColor;
                  })";
            postFragmentSource = replace(postFragmentSource, "%fogMinDistance%", std::to_string(fogMinDistance));
            postFragmentSource = replace(postFragmentSource, "%fogMaxDistance%", std::to_string(fogMaxDistance));
            postFragmentSource = replace(postFragmentSource, "%fogPower%", std::to_string(fogPower));
            material.setColors(ofFloatColor::white, ofFloatColor::white, ofFloatColor::white, ofFloatColor::black);
            material.setPostFragment(postFragmentSource);
        }

        void WireframeFillRenderer::setup(){
            compileShader();
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
        }

        void WireframeFillRenderer::draw(const ofVbo & geometry, size_t offset, size_t numVertices) const{
            ofDisableDepthTest();
            if (fill) {
                shaderFill.begin();
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

        using namespace glm;
        float fog(float dist, float minDist, float maxDist, float power) {
            dist = pow(dist, power);
            minDist = pow(minDist, power);
            maxDist = pow(maxDist, power);
            float invDistanceToCamera = clamp(1 - (dist - minDist) / maxDist, 0.f, 1.f);
            if (dist > minDist) {
                return invDistanceToCamera;
            }
            else {
                return 1;
            }
        }

        std::vector<float> WireframeFillRenderer::getFogFunctionPlot(size_t numberOfPoints) const {
            std::vector<float> plot(numberOfPoints);
            for (size_t i = 0; i < numberOfPoints; i++) {
                float distanceToCamera = i/float(numberOfPoints) * 10.;
                plot[i] = fog(distanceToCamera, fogMinDistance, fogMaxDistance, fogPower);
            }
            return plot;
        }
    }
}
