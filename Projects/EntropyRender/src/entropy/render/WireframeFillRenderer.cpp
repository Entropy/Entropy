#include "WireframeFillRenderer.h"
#include "ofGraphics.h"
#include <regex>
#include <random>
#include "ofPolyline.h"
#include "entropy/Helpers.h"

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

		ofMesh circle_arc_grid ( float r, glm::vec2 center, float arcBegin, float arcEnd, int samples )
		{
		  double aj;
		  int j;
		  double pi = 3.141592653589793;
		  ofMesh mesh;
		  mesh.setMode(OF_PRIMITIVE_POINTS);
		  mesh.getVertices().resize( samples );

		  for ( j = 0; j < samples; j++ )
		  {
			aj = ( ( double ) ( samples - j - 1 ) * arcBegin
				 + ( double ) (     j     ) * arcEnd )
				 / ( double ) ( samples     - 1 );

			mesh.getVertices()[j].x = center.x + r * cos ( aj * pi / 180.0 );
			mesh.getVertices()[j].y = center.y + r * sin ( aj * pi / 180.0 );
		  }

		  return mesh;
		}


		float sign (glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		bool pointInTriangle (glm::vec2 pt, glm::vec2 v1, glm::vec2 v2, glm::vec2 v3)
		{
			bool b1, b2, b3;

			b1 = sign(pt, v1, v2) <= 0.0f;
			b2 = sign(pt, v2, v3) <= 0.0f;
			b3 = sign(pt, v3, v1) <= 0.0f;

			return ((b1 == b2) && (b2 == b3));
		}

		bool isInside(ofPolyline polygon, glm::vec2 p){
			for(auto i=0;i<polygon.size(); i++){
				ofPolyline tri;
				tri.addVertex(glm::vec3(0));
				tri.addVertex(polygon[i]);
				tri.addVertex(polygon[(i+1) % polygon.size()]);
				if(pointInTriangle(p, tri[0].xy(), tri[1].xy(), tri[2].xy())){
					return true;
				}
			}
			return false;
		}

		bool inContour(const ofPolyline & poly, glm::vec2 p){
			for(auto i=0;i<poly.size();i++){
				auto p1 = poly[i];
				auto p2 = poly[(i + 1) % poly.size()];
				bool inLine = abs((p.x - p1.x) / (p2.x - p1.x) - (p.y - p1.y) / (p2.y - p1.y))<0.001;
				if(inLine){
					auto inX = false;
					auto inY = false;
					if(p1.x < p2.x){
						inX = p1.x <= p.x && p.x <= p2.x;
					}else{
						inX = p2.x <= p.x && p.x <= p1.x;
					}
					if(p1.y < p2.y){
						inY = p1.y <= p.y && p.y <= p2.y;
					}else{
						inY = p2.y <= p.y && p.y <= p1.y;
					}
					if(inX && inY) return true;
				}
			}
			return false;
		}

		ofMesh generateBokehShape(int dofSamples, int bokehsides){
			ofMesh bokehshape;
			bokehshape.setMode(OF_PRIMITIVE_POINTS);
//			if(dofSamples>1){
//				auto rings = dofSamples / bokehsides;
//				for (auto ring=0; ring<rings; ring++){
//					auto bias = 1.0 / float(bokehsides);
//					auto radius = (float(ring) + bias) / (float(rings) + bias);
//					auto points = ring * bokehsides;
//					for (auto pt=0; pt<points; pt++){
//						auto phi = 2.0 * M_PI * float(pt) / float(points);
//						auto x = cos(phi) * radius;
//						auto y = sin(phi) * radius;
//						bokehshape.addVertex({x,y,0});
//					}
//				}
//			}else{
//				bokehshape.addVertex({0,0,0});
//			}

			if(dofSamples>1){
				ofPolyline polygon;
				polygon.arc(glm::vec3(0), 1, 1, 0, 360, true, bokehsides);
				polygon.close();

				glm::vec2 min = polygon[0].xy(), max = polygon[0].xy();
				for(auto & p: polygon){
					if(p.x<min.x) min.x = p.x;
					if(p.y<min.y) min.y = p.y;

					if(p.x>max.x) max.x = p.x;
					if(p.y>max.y) max.y = p.y;
				}

				auto range = max - min;
				auto rows = sqrt(dofSamples);
				auto cols = rows;
				auto step = range / float(rows-1);
				auto p = min;
				for(auto y=0;y<rows;y++){
					for(auto x=0;x<cols;x++){
						if(polygon.inside(p.x, p.y) || inContour(polygon, p)){
							bokehshape.addVertex({p.x, p.y, 0});
						}
						p.x += step.x;
					}
					p.y += step.y;
					p.x = min.x;
				}
			}else{
				bokehshape.addVertex({0,0,0});
			}
			return bokehshape;
		}

		glm::vec2 mirror(glm::vec2 p, glm::vec2 l0, glm::vec2 l1){
		   auto dx  = l1.x - l0.x;
		   auto dy  = l1.y - l0.y;

		   auto a   = (dx * dx - dy * dy) / (dx * dx + dy*dy);
		   auto b   = 2 * dx * dy / (dx*dx + dy*dy);

		   auto x2  = round(a * (p.x - l0.x) + b*(p.y - l0.y) + l0.x);
		   auto y2  = round(b * (p.x - l0.x) - a*(p.y - l0.y) + l0.y);


		   return {x2,y2};
		}

		ofMesh generateBokehShape2(int dofSamples, int bokehsides){
			const float range_from  = 0;
			const float range_to    = 1;
			std::random_device                  rand_dev;
			std::mt19937                        generator(rand_dev());
			std::uniform_real_distribution<float>  distr(range_from, range_to);

			ofMesh bokehshape;
			bokehshape.setMode(OF_PRIMITIVE_POINTS);
			bokehshape.addVertex({0,0,0});
			if(dofSamples>1){
				ofPolyline polygon;
				polygon.arc(glm::vec3(0), 1, 1, 0, 360, true, bokehsides);
				polygon.close();
				for(auto i=0;i<bokehsides; i++){
					ofPolyline tri;
					tri.addVertex(glm::vec3(0));
					tri.addVertex(polygon[i]);
					tri.addVertex(polygon[(i+1) % bokehsides]);
					for(auto j = 0; j < dofSamples / bokehsides; j++){
						auto a1 = distr(generator);
						auto a2 = distr(generator);
						auto p = (1.f - sqrt(a1)) * tri[0] + sqrt(a1) * (1 - a2) * tri[1] + sqrt(a1) * a2 * tri[2];
						bokehshape.addVertex(p);
					}
				}
			}
			return bokehshape;
		}

		void WireframeFillRenderer::setup(float sceneSize){
			this->sceneSize = sceneSize;
			shaderSettings.shaderFiles[GL_VERTEX_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.vert";
			shaderSettings.shaderFiles[GL_FRAGMENT_SHADER] = GetShadersPath(Module::Renderers) / "wireframeFillRender.frag";
			shaderSettings.bindDefaults = true;
			shaderSettings.intDefines["FOG_ENABLED"] = parameters.fogEnabled;
			shaderSettings.intDefines["SHADE_NORMALS"] = parameters.shadeNormals;

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
					localColor *= accumValue;
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
				uniform float accumValue;
			)";
			material.setup(settings);

			settings.postFragment = "#define SPHERICAL_CLIP 1\n" + settings.postFragment;
			materialSphericalClip.setup(settings);

			listeners.push_back((parameters.fogEnabled.newListener([&](bool & enabled){
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

			listeners.push_back(parameters.dofSamples.newListener([&](int & dofSamples){
				bokehshape = generateBokehShape(dofSamples, parameters.bokehsides);
				modelview.resize(bokehshape.getVertices().size());
				mvp.resize(bokehshape.getVertices().size());
			}));

			listeners.push_back(parameters.bokehsides.newListener([&](int & bokehsides){
				bokehshape = generateBokehShape(parameters.dofSamples, bokehsides);
				modelview.resize(bokehshape.getVertices().size());
				mvp.resize(bokehshape.getVertices().size());
			}));

			bokehshape = generateBokehShape(parameters.dofSamples, parameters.bokehsides);
			modelview.resize(bokehshape.getVertices().size());
			mvp.resize(bokehshape.getVertices().size());

		}

		const ofMesh & WireframeFillRenderer::getBokehShape() const{
			return bokehshape;
		}

		void WireframeFillRenderer::resize(float width, float height){
			this->blobMask.setup(width, height, 0.25f * sceneSize);
		}

		void WireframeFillRenderer::drawDebug(){
			this->blobMask.getMaxDepthMask().draw(0,0);
		}

		void WireframeFillRenderer::drawWithDOF(ofCamera & camera, std::function<void(float accumValue, glm::mat4 projection, glm::mat4 modelview)> drawFunc) const{
			auto accumValue = 1.0 / float(bokehshape.getVertices().size());

			auto projection = ofGetCurrentOrientationMatrix() * camera.getProjectionMatrix();
			auto object = camera.getPosition() - camera.getZAxis() * parameters.dofDistance.get() * sceneSize;
			auto eye = camera.getPosition();
			auto up = camera.getYAxis();
			auto right = glm::normalize(glm::cross(object - eye, up));
			auto numSamples = bokehshape.getVertices().size();
			for(size_t i = 0; i < numSamples; i++){
				auto p = bokehshape.getVertices()[i];
				glm::vec3 bokeh = right * p.x + up * p.y;
				auto modelview = glm::lookAt(eye + bokeh * parameters.dofAperture.get() * sceneSize, object, up);
				drawFunc(accumValue, projection, modelview);
			}
		}

		void WireframeFillRenderer::draw(const ofVbo & geometry, size_t offset, size_t numVertices, GLenum mode, ofCamera & camera, const glm::mat4 & model) const{
			DrawSettings settings;
			settings.offset = offset;
			settings.numVertices = numVertices;
			settings.mode = mode;
			settings.camera = &camera;
			settings.model = model;
			settings.parameters = parameters;
			draw(geometry, settings);
		}

		void WireframeFillRenderer::draw(const ofVbo & geometry, WireframeFillRenderer::DrawSettings settings) const{
			auto parameters = settings.parameters;
			if(parameters.wobblyClip){
				this->blobMask.updateWith(*settings.camera);
			}

			ofDisableDepthTest();
			//ofEnableBlendMode(OF_BLENDMODE_ADD);
//			glEnable(GL_BLEND);
//			glBlendFunc(GL_ONE, GL_ONE);
			ofShader shaderFill;
			ofShader shaderWireframe;
			if(parameters.sphericalClip){
				shaderFill = this->shaderFillSphere;
				shaderWireframe = this->shaderWireframeSphere;
			}else{
				shaderFill = this->shaderFill;
				shaderWireframe = this->shaderWireframe;
			}
			auto numSamples = parameters.enableDOF ? bokehshape.getVertices().size() : 1;
			auto accumValue = 1.0 / float(numSamples);

			shaderFill.begin();
			shaderFill.setUniform1f("accumValue", accumValue);
			shaderFill.setUniform1f("fogStartDistance", parameters.fogStartDistance);
			shaderFill.setUniform1f("fogMinDistance", parameters.fogMinDistance);
			shaderFill.setUniform1f("fogMaxDistance", parameters.fogMaxDistance);
			shaderFill.setUniform1f("fogPower", parameters.fogPower);
			shaderFill.setUniform1f("fadeEdge0", parameters.fadeEdge0);
			shaderFill.setUniform1f("fadeEdge1", parameters.fadeEdge1);
			shaderFill.setUniform1f("fadePower", parameters.fadePower);
			shaderFill.setUniform1f("alpha", parameters.fillAlpha * parameters.alphaFactor);
			shaderFill.setUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
			shaderFill.setUniform1f("screenW", ofGetViewportWidth());
			shaderFill.setUniform1f("screenH", ofGetViewportHeight());
			shaderFill.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
			shaderFill.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
			shaderFill.end();

			shaderWireframe.begin();
			shaderWireframe.setUniform1f("accumValue", accumValue);
			shaderWireframe.setUniform1f("fogStartDistance", parameters.fogStartDistance);
			shaderWireframe.setUniform1f("fogMinDistance", parameters.fogMinDistance);
			shaderWireframe.setUniform1f("fogMaxDistance", parameters.fogMaxDistance);
			shaderWireframe.setUniform1f("fogPower", parameters.fogPower);
			shaderWireframe.setUniform1f("fadeEdge0", parameters.fadeEdge0);
			shaderWireframe.setUniform1f("fadeEdge1", parameters.fadeEdge1);
			shaderWireframe.setUniform1f("fadePower", parameters.fadePower);
			shaderWireframe.setUniform1f("alpha", parameters.wireframeAlpha * parameters.alphaFactor);
			shaderWireframe.setUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
			shaderWireframe.setUniform1f("screenW", ofGetViewportWidth());
			shaderWireframe.setUniform1f("screenH", ofGetViewportHeight());
			shaderWireframe.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
			shaderWireframe.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
			shaderWireframe.end();

			auto * material = (parameters.clip && parameters.sphericalClip) ? &this->materialSphericalClip : &this->material;
			if(parameters.useLights){
				material->setCustomUniform1f("accumValue", accumValue);
				material->setCustomUniform1f("fogStartDistance", parameters.fogStartDistance);
				material->setCustomUniform1f("fogMinDistance", parameters.fogMinDistance);
				material->setCustomUniform1f("fogMaxDistance", parameters.fogMaxDistance);
				material->setCustomUniform1f("fogPower", parameters.fogPower);
				material->setCustomUniform1f("fadeEdge0", parameters.fadeEdge0);
				material->setCustomUniform1f("fadeEdge1", parameters.fadeEdge1);
				material->setCustomUniform1f("fadePower", parameters.fadePower);
				material->setCustomUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
				material->setCustomUniform1f("screenW", ofGetViewportWidth());
				material->setCustomUniform1f("screenH", ofGetViewportHeight());
				material->setCustomUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
				material->setCustomUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
			}

			auto & camera = *settings.camera;
			auto & model = settings.model;
			auto mode = settings.mode;
			auto offset = settings.offset;
			auto numVertices = settings.numVertices;
			auto projection = ofGetCurrentOrientationMatrix() * camera.getProjectionMatrix();

			if(parameters.enableDOF){
				auto object = camera.getPosition() - camera.getZAxis() * parameters.dofDistance.get() * sceneSize;
				auto eye = camera.getPosition();
				auto up = camera.getYAxis();
				auto right = glm::normalize(glm::cross(object - eye, up));
				for(size_t i = 0; i < numSamples; i++){
					auto p = bokehshape.getVertices()[i];
					glm::vec3 bokeh = right * p.x + up * p.y;
					modelview[i] = glm::lookAt(eye + bokeh * parameters.dofAperture.get() * sceneSize, object, up) * model;
					mvp[i] = projection * modelview[i];
				}
			}else{
				modelview[0] = camera.getModelViewMatrix() * model;
				mvp[0] = projection * modelview[0];
			}

			if (parameters.fill) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				shaderFill.begin();
				for(size_t i = 0; i < numSamples; i++){
					shaderFill.setUniformMatrix4f("modelViewMatrix", modelview[i]);
					shaderFill.setUniformMatrix4f("modelViewProjectionMatrix", mvp[i]);
					geometry.draw(mode, offset, numVertices);
				}
				shaderFill.end();
				if(parameters.useLights){
					material->begin();
					for(size_t i = 0; i < numSamples; i++){
						material->setCustomUniformMatrix4f("modelViewMatrix", modelview[i]);
						material->setCustomUniformMatrix4f("modelViewProjectionMatrix", mvp[i]);
						geometry.draw(mode, offset, numVertices);
					}
					material->end();
				}
			}

			if (parameters.wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				if(parameters.useLights){
					auto * material = (parameters.clip && parameters.sphericalClip) ? &this->materialSphericalClip : &this->material;
					material->begin();
					for(size_t i = 0; i < numSamples; i++){
						material->setCustomUniformMatrix4f("modelViewMatrix", modelview[i]);
						material->setCustomUniformMatrix4f("modelViewProjectionMatrix", mvp[i]);
						geometry.draw(mode, offset, numVertices);
					}
					material->end();
				}else{
					shaderWireframe.begin();
					for(size_t i = 0; i < numSamples; i++){
						shaderWireframe.setUniformMatrix4f("modelViewMatrix", modelview[i]);
						shaderWireframe.setUniformMatrix4f("modelViewProjectionMatrix", mvp[i]);
						geometry.draw(mode, offset, numVertices);
					}
					shaderWireframe.end();
				}
			}
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
//			ofEnableBlendMode(OF_BLENDMODE_ALPHA);
		}

		void WireframeFillRenderer::drawElements(const ofVbo & geometry, size_t offset, size_t numIndices, GLenum mode, ofCamera & camera) const{
			if(parameters.wobblyClip){
				this->blobMask.updateWith(camera);
			}

			ofDisableDepthTest();
			ofShader shaderFill;
			ofShader shaderWireframe;
			if(parameters.sphericalClip && parameters.clip){
				shaderFill = this->shaderFillSphere;
				shaderWireframe = this->shaderWireframeSphere;
			}else{
				shaderFill = this->shaderFill;
				shaderWireframe = this->shaderWireframe;
			}
			if (parameters.fill) {
				shaderFill.begin();
				shaderFill.setUniform1f("fogStartDistance", parameters.fogStartDistance);
				shaderFill.setUniform1f("fogMinDistance", parameters.fogMinDistance);
				shaderFill.setUniform1f("fogMaxDistance", parameters.fogMaxDistance);
				shaderFill.setUniform1f("fogPower", parameters.fogPower);
				shaderFill.setUniform1f("fadeEdge0", parameters.fadeEdge0);
				shaderFill.setUniform1f("fadeEdge1", parameters.fadeEdge1);
				shaderFill.setUniform1f("fadePower", parameters.fadePower);
				shaderFill.setUniform1f("alpha", parameters.fillAlpha * parameters.alphaFactor);
				shaderFill.setUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
				shaderFill.setUniform1f("screenW", ofGetViewportWidth());
				shaderFill.setUniform1f("screenH", ofGetViewportHeight());
				shaderFill.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
				shaderFill.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				geometry.drawElements(mode, numIndices, offset);
				shaderFill.end();
			}

			if (parameters.wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

				if(parameters.useLights){
					auto * material = (parameters.clip && parameters.sphericalClip) ? &this->materialSphericalClip : &this->material;
					material->begin();
					material->setCustomUniform1f("fogStartDistance", parameters.fogStartDistance);
					material->setCustomUniform1f("fogMinDistance", parameters.fogMinDistance);
					material->setCustomUniform1f("fogMaxDistance", parameters.fogMaxDistance);
					material->setCustomUniform1f("fogPower", parameters.fogPower);
					material->setCustomUniform1f("fadeEdge0", parameters.fadeEdge0);
					material->setCustomUniform1f("fadeEdge1", parameters.fadeEdge1);
					material->setCustomUniform1f("fadePower", parameters.fadePower);
					material->setCustomUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
					material->setCustomUniform1f("screenW", ofGetViewportWidth());
					material->setCustomUniform1f("screenH", ofGetViewportHeight());
					material->setCustomUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					material->setCustomUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.drawElements(mode, numIndices, offset);
					material->end();
				}else{
					shaderWireframe.begin();
					shaderWireframe.setUniform1f("fogStartDistance", parameters.fogStartDistance);
					shaderWireframe.setUniform1f("fogMinDistance", parameters.fogMinDistance);
					shaderWireframe.setUniform1f("fogMaxDistance", parameters.fogMaxDistance);
					shaderWireframe.setUniform1f("fogPower", parameters.fogPower);
					shaderWireframe.setUniform1f("fadeEdge0", parameters.fadeEdge0);
					shaderWireframe.setUniform1f("fadeEdge1", parameters.fadeEdge1);
					shaderWireframe.setUniform1f("fadePower", parameters.fadePower);
					shaderWireframe.setUniform1f("alpha", parameters.wireframeAlpha * parameters.alphaFactor);
					shaderWireframe.setUniform1f("wobblyClip", parameters.wobblyClip && parameters.clip);
					shaderWireframe.setUniform1f("screenW", ofGetViewportWidth());
					shaderWireframe.setUniform1f("screenH", ofGetViewportHeight());
					shaderWireframe.setUniformTexture("minDepthMask", this->blobMask.getMinDepthMask(), 0);
					shaderWireframe.setUniformTexture("maxDepthMask", this->blobMask.getMaxDepthMask(), 1);
					geometry.drawElements(mode, numIndices, offset);
					shaderWireframe.end();
				}
			}
		}

		std::vector<float> WireframeFillRenderer::getFogFunctionPlot(size_t numberOfPoints) const {
			std::vector<float> plot(numberOfPoints);
			for (size_t i = 0; i < numberOfPoints; i++) {
				float distanceToCamera = i/float(numberOfPoints) * 10.;
				plot[i] = fog(distanceToCamera, parameters.fogStartDistance, parameters.fogMinDistance, parameters.fogMaxDistance, parameters.fogPower);
			}
			return plot;
		}
	}
}
