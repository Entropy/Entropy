#ifndef BLOBMASK_H
#define BLOBMASK_H

#include "ofFbo.h"
#include "ofShader.h"
#include "ofCamera.h"

class BlobMask
{
public:
	void setup(float width, float height, float radius);
	void updateWith(ofCamera & camera);
	ofTexture & getMinDepthMask();
	ofTexture & getMaxDepthMask();
private:
	struct Sphere{
		Sphere(glm::vec3 pos, float radius)
			:pos(pos)
			,radius(radius){}
		glm::vec3 pos;
		float radius;
	};
	std::vector<Sphere> blob;
	ofFbo minDepthFbo;
	ofFbo maxDepthFbo;
	ofShader minMaxDepthShader;
	float radius;
};

#endif // BLOBMASK_H
