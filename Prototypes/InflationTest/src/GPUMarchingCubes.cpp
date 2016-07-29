#include "GPUMarchingCubes.h"
#include "mcTables.h"

void GPUMarchingCubes::setup(){
	resolutionListener = resolution.newListener([&](int res){
		std::vector<glm::vec3> vertices(res*res*res);
		for(int z = 0, i = 0; z < res; z++){
			for(int y = 0; y < res; y++){
				for(int x = 0; x < res; x++, i++){
                    vertices[i] = glm::vec3{float(x), float(y), float(z)} / float(res) - glm::vec3(0.5f);
				}
			}
		}
		vbo.setVertexData(vertices.data(), vertices.size(), GL_STATIC_DRAW);
	});
    resolution = 128;

	triTableTex.allocate(16, 256, GL_R8I, false, GL_RED_INTEGER, GL_BYTE);
	triTableTex.loadData(&triTable[0][0], 16, 256, GL_RED_INTEGER);
	triTableTex.setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);

    shader.load("shaders/passthrough_vert.glsl", "shaders/normalShader.frag", "shaders/marching_cubes_geom.glsl");
}


void GPUMarchingCubes::draw(ofxTexture3d & isoLevels, float threshold){
	shader.begin();
	shader.setUniformTexture("dataFieldTex", isoLevels.texData.textureTarget, isoLevels.texData.textureID, 0);
	shader.setUniformTexture("triTableTex", triTableTex, 1);
	shader.setUniform1f("isolevel", threshold);
	shader.setUniform1f("half_resolution", resolution / 2);
	shader.setUniform1f("resolution", resolution);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	vbo.draw(GL_POINTS, 0, resolution*resolution*resolution);
	shader.end();
}
