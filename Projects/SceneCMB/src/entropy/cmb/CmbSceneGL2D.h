#pragma once

#include "entropy/cmb/Constants.h"
#ifdef COMPUTE_GL_2D

#include "ofxFbo.h"

#include "CmbScene.h"

namespace ent
{
	class CmbSceneGL2D
		: public CmbScene
	{
	public:
		void setup() override;
		void draw() override;

	protected:
		void addDrop() override;
		void stepRipple() override;
		void copyResult() override;

		ofShader m_shader;
		ofVboMesh m_mesh;

		ofTexture m_textures[3]; 
		ofxFbo m_fbos[3];
	};
}
#endif // COMPUTE_GL_2D
