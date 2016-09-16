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
			enum class FilterMode
			{
				Linear,
				Nearest
			};

			PoolGL3D();

			void init() override;
			void resize() override;

			void reset() override;
			void update(double dt) override;
			void draw() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			ofParameter<int> filterMode{ "Filter Mode", static_cast<int>(FilterMode::Linear), static_cast<int>(FilterMode::Nearest), static_cast<int>(FilterMode::Linear) };
			ofParameter<float> volumeSize{ "Volume Size", 800.0f, 512.0f, 1920.0f };

			const ofxTexture & getTexture() const;

		protected:
			void addDrop() override;
			void stepRipple() override;
			void copyResult() override;
			void mixFrames(float pct) override;

			ofShader dropShader;
			ofShader rippleShader;
#if USE_COPY_SHADER
			ofShader copyShader;
#endif
			ofShader mixShader;

			ofVboMesh mesh;

#if USE_TEX_ARRAY
			ofxTextureArray textures[3];
			ofxVolumetricsArray volumetrics;
#else
			ofxTexture3d textures[3];
			ofxVolumetrics3D volumetrics;
			ofShader volumetricsShader;
#endif
			ofxFbo fbos[3];

			Bursts bursts;
		};
	}
}

#endif // COMPUTE_GL_3D

