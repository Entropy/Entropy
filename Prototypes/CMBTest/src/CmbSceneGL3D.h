#pragma once

#include "ofxFbo.h"
#include "ofxVolumetricsArray.h"

#include "CmbScene.h"

namespace ent
{
	class CmbSceneGL3D
		: public CmbScene
	{
	public:
		void setup() override;
		void draw() override;

	protected:
		void addDrop() override;
		void stepRipple() override;
		void copyResult() override;

		ofShader m_dropShader;
		ofShader m_rippleShader;
		ofShader m_copyShader;
		ofVboMesh m_mesh;

		ofxTextureArray m_textures[3];
		ofxFbo m_fbos[3];
		ofxVolumetricsArray m_volumetrics;
	};
}
