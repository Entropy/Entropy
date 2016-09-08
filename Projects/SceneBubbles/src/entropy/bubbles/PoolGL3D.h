#pragma once

#include "entropy/bubbles/Constants.h"
#ifdef COMPUTE_GL_3D

#include "ofxFbo.h"
#include "ofxVolumetricsArray.h"

#include "PoolBase.h"

namespace entropy
{
	namespace bubbles
	{
		class PoolGL3D
			: public PoolBase
		{
		public:
			PoolGL3D();

			void setup() override;

			void reset() override;
			void draw() override;

			ofParameter<int> filterMode{ "Filter Mode", GL_LINEAR, GL_NEAREST, GL_LINEAR };
			ofParameter<float> volumeSize{ "Volume Size", 800.0f, 512.0f, 1920.0f };

		protected:
			void addDrop() override;
			void stepRipple() override;
			void copyResult() override;

			ofShader dropShader;
			ofShader rippleShader;
			ofShader copyShader;
			ofVboMesh mesh;

			ofBufferObject copyBuffer;

			ofxTextureArray textures[3];
			ofxFbo fbos[3];
			ofxVolumetricsArray volumetrics;
		};
	}
}

#endif // COMPUTE_GL_3D

