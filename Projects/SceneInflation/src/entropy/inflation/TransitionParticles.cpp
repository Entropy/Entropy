#include "TransitionParticles.h"
#include "ofxObjLoader.h"

void TransitionParticles::setup(){
	ofMesh mesh;
	ofxObjLoader::load("models/particle.obj", mesh);
	model.setMesh(mesh, GL_STATIC_DRAW);
	modelNumVertices = mesh.getNumVertices();

	ofShader::TransformFeedbackSettings settings;
	settings.shaderFiles[GL_VERTEX_SHADER] = "shaders/transition_particle.vert";
	settings.varyingsToCapture = { "v_position", "v_color" };
	shader.setup(settings);
	computeShader.loadCompute("shaders/compute_particles.glsl");
	glGenQueries(1, &numVerticesQuery);

	//feedbackBuffer.allocate(41535936 * 2, GL_STATIC_DRAW);
	//positions.setVertexBuffer(feedbackBuffer, 4, sizeof(glm::vec4) * 2, 0);
	//positions.setColorBuffer(feedbackBuffer, sizeof(glm::vec4) * 2, sizeof(glm::vec4));
}

void TransitionParticles::setTotalVertices(int totalVertices) {
	this->totalVertices = totalVertices;
	feedbackBuffer.allocate(totalVertices / every * modelNumVertices * sizeof(glm::vec4) * 2, GL_STATIC_DRAW);
	positions.setVertexBuffer(feedbackBuffer, 4, sizeof(glm::vec4) * 2, 0);
	positions.setColorBuffer(feedbackBuffer, sizeof(glm::vec4) * 2, sizeof(glm::vec4));
}

void TransitionParticles::update(const ofBufferObject & blobs, const ofxTexture3d & noiseField, float now){
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

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, numVerticesQuery);
	shader.beginTransformFeedback(GL_TRIANGLES, feedbackBuffer);
	blobs.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	shader.setUniform1f("every", every);
	shader.setUniform1f("scale", scale);
	shader.setUniform4f("particleColor", color);
	model.drawInstanced(GL_TRIANGLES, 0, model.getNumVertices(), numParticles);
	shader.endTransformFeedback(feedbackBuffer);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(numVerticesQuery, GL_QUERY_RESULT, &numPrimitives);
}

int TransitionParticles::getNumVertices() const {
	return numPrimitives * 3;
}

const ofVbo & TransitionParticles::getVbo() const {
	return positions;
}