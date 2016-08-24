#ifndef GPUMARCHINGCUBES_H
#define GPUMARCHINGCUBES_H

#include "ofVbo.h"
#include "ofShader.h"
#include "ofParameter.h"
#include "ofxTexture3d.h"
#include "ofEvents.h"

namespace entropy
{
	namespace inflation
	{
		class GPUMarchingCubes
		{
		public:
            void setup(size_t maxMemory);
            void update(ofxTexture3d & isoLevels);
            const ofVbo & getGeometry();
            size_t getNumVertices() const;
            size_t getBufferSize() const;
            size_t getVertexStride() const;


            ofParameter<int> resolution{ "Resolution", 64, 1, 512 };
            ofParameter<int> subdivisions{ "subdivisions", 0, 0, 4 };
            ofParameter<float> isoLevel{ "IsoLevel", 0.3f, 0.0f, 1.0f };
            ofParameter<bool> shadeNormals{ "Shade Normals", false };

			ofParameterGroup parameters{
				"Marching Cubes",
                resolution,
                isoLevel,
                shadeNormals,
                subdivisions,
			};

        private:
            size_t getFeedbackBufferSize() const;
            ofBufferObject bufferFeedback;
            ofVbo vbo, vboFeedback;
            ofShader shader;
			ofTexture triTableTex;
            ofEventListener resolutionListener, subdivisionsListener, shadeNormalsListener;
            GLuint numVerticesQuery;
            GLuint numPrimitives;
            size_t feedbackBufferSize;
            size_t maxMemorySize;
		};
	}
}

#endif // GPUMARCHINGCUBES_H
