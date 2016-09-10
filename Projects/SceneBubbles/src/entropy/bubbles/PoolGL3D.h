#pragma once

#include "entropy/bubbles/Constants.h"
#ifdef COMPUTE_GL_3D

#include "ofxFbo.h"

#if USE_TEX_ARRAY
#include "ofxVolumetricsArray.h"
#else
#include "ofxVolumetrics3D.h"
#endif

#include "PoolBase.h"
#include "Bursts.h"

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
			void update(double dt) override;
			void draw() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			ofParameter<int> filterMode{ "Filter Mode", GL_LINEAR, GL_NEAREST, GL_LINEAR };
			ofParameter<float> volumeSize{ "Volume Size", 800.0f, 512.0f, 1920.0f };

			const ofxTexture & getTexture() const;

		protected:
			void addDrop() override;
			void stepRipple() override;
			void copyResult() override;

			ofShader dropShader;
			ofShader rippleShader;
			ofShader copyShader;

			ofVboMesh mesh;

			ofBufferObject copyBuffer;

#if USE_TEX_ARRAY
			ofxTextureArray textures[3];
			ofxVolumetricsArray volumetrics;
#else
			ofxTexture3d textures[3];
			ofxVolumetrics3D volumetrics;
#endif
			ofxFbo fbos[3];

			Bursts bursts;
		};
	}
}

#endif // COMPUTE_GL_3D

