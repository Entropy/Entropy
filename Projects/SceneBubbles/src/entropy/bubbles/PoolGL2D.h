#pragma once

#include "entropy/bubbles/Constants.h"
#ifdef COMPUTE_GL_2D

#include "ofxFbo.h"

#include "PoolBase.h"

namespace entropy
{
	namespace bubbles
	{
		class PoolGL2D
			: public PoolBase
		{
		public:
			PoolGL2D();

			void init() override;
			void resize() override;

			void reset() override;
			void draw() override;

		protected:
			void addDrop() override;
			void stepRipple() override;
			void copyResult() override;
			void mixFrames(float pct) override;
			void setDrawTextureIndex(int idx) override;

			ofShader rippleShader;
			ofShader mixShader;
			ofVboMesh mesh;

			ofTexture textures[3];
			ofxFbo fbos[3];
			int drawIdx;
		};
	}
}
#endif // COMPUTE_GL_2D
