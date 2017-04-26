#include "BlobMask.h"
#include "ofGraphics.h"
#include "of3dGraphics.h"

void BlobMask::setup(float width, float height, float radius){
	cout << "allocating blob mask with " << width << " x " << height << endl;
	this->radius = radius;

	for(int i=0;i<100;i++){
		blob.emplace_back(glm::vec3{ofRandom(-radius,radius), ofRandom(-radius,radius), ofRandom(-radius,radius)}, ofRandom(radius));
	}

	{
		ofFbo::Settings settings;
		settings.width = width;
		settings.height = height;
		settings.depthStencilAsTexture = true;
		settings.depthStencilInternalFormat = GL_DEPTH_COMPONENT32;
		settings.useStencil = false;
		settings.useDepth = true;
		settings.numColorbuffers = 0;
		settings.textureTarget = GL_TEXTURE_2D;
		minDepthFbo.allocate(settings);
		maxDepthFbo.allocate(settings);
		minDepthFbo.getDepthTexture().setRGToRGBASwizzles(true);
		maxDepthFbo.getDepthTexture().setRGToRGBASwizzles(true);
	}

	{
		ofShader::Settings settings;
		settings.shaderSources[GL_VERTEX_SHADER] = R"(
			#version 330
			uniform mat4 modelViewProjectionMatrix;

			in vec4 position;

			void main(void){
			   gl_Position = modelViewProjectionMatrix * position;
			}
		)";

		settings.shaderSources[GL_FRAGMENT_SHADER] = R"(
			#version 330
			void main(void){
				//gl_FragDepth = gl_FragCoord.z / gl_FragCoord.w;
			}
		)";

		minMaxDepthShader.setup(settings);
	}

}

void BlobMask::updateWith(ofCamera & camera){
	for(auto & sphere: blob){
		sphere.radius = ofNoise(sphere.pos.x, sphere.pos.y, sphere.pos.z, ofGetElapsedTimef() * 0.5) * this->radius;
	}

	ofEnableDepthTest();
	minMaxDepthShader.begin();

	minDepthFbo.begin();
	minDepthFbo.clearDepthBuffer(0.f);
	glDepthFunc(GL_GREATER);
	camera.begin();
	for(auto & sphere: blob){
		ofDrawSphere(sphere.pos, sphere.radius);
	}
	camera.end();
	minDepthFbo.end();


	maxDepthFbo.begin();
	maxDepthFbo.clearDepthBuffer(1.f);
	glDepthFunc(GL_LESS);
	camera.begin();
	for(auto & sphere: blob){
		ofDrawSphere(sphere.pos, sphere.radius);
	}
	camera.end();
	maxDepthFbo.end();

	minMaxDepthShader.end();
	ofDisableDepthTest();


	/*ofEnableDepthTest();
	camera.begin();
	for(auto & sphere: blob){
		ofDrawSphere(sphere.pos, sphere.radius);
	}
	camera.end();
	ofDisableDepthTest();*/
}

ofTexture & BlobMask::getMinDepthMask(){
	return minDepthFbo.getDepthTexture();
}

ofTexture & BlobMask::getMaxDepthMask(){
	return maxDepthFbo.getDepthTexture();
}
