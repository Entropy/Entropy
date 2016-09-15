#include "TransitionParticles.h"
#include "ofxObjLoader.h"

void TransitionParticles::setup(){
	ofMesh mesh;
	ofxObjLoader::load("models/particle.obj", mesh);
	model.setMesh(mesh, GL_STATIC_DRAW);

	shader.load("shaders/transition_particle.vert", "shaders/transition_particle.frag");
	computeShader.loadCompute("shaders/compute_particles.glsl");
}

void TransitionParticles::draw(const ofBufferObject & blobs, const ofxTexture3d & noiseField, float now){
	auto numParticles = totalVertices / every;

	computeShader.begin();
	computeShader.setUniform1f("every", every);
	computeShader.setUniform1f("now", now);
	computeShader.setUniform1f("dt",ofGetLastFrameTime()*speed);
	computeShader.setUniform1f("repulsionForce", repulsion);
	computeShader.setUniform1f("attractionForce", attraction);
	computeShader.setUniform1f("elapsedTime",now);
	computeShader.setUniform1f("bufferSize", totalVertices);
	computeShader.setUniform1f("noiseSize", noiseField.texData.width);
	computeShader.setUniform1f("frameNum", ofGetFrameNum());
	computeShader.setUniformTexture("noiseField", GL_TEXTURE_3D, noiseField.texData.textureID, 0);
	computeShader.dispatchCompute(numParticles / 1024 + 1, 1, 1);
	computeShader.end();

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	shader.begin();
	blobs.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	shader.setUniform1f("every", every);
	shader.setUniform1f("scale", scale);
	shader.setUniform4f("particleColor", color);
	model.drawInstanced(GL_TRIANGLES, 0, model.getNumVertices(), numParticles);
	shader.end();
}
