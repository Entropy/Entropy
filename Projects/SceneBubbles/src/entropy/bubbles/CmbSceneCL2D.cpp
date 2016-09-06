#include "CmbSceneCL2D.h"

#ifdef COMPUTE_CL_2D

namespace ent
{
	//--------------------------------------------------------------
	void CmbSceneCL2D::setup()
	{
		CmbScene::setup();

		m_openCL.setupFromOpenGL();

		// Allocate the textures and buffers.
		for (int i = 0; i < 3; ++i) {
			m_clImages[i].initWithTexture(m_dimensions.x, m_dimensions.y, GL_RGBA16F);
		}

		// Load program and kernels.
		m_openCL.loadProgramFromFile("cl/cmb.cl");

		m_dropKernel = m_openCL.loadKernel("drop2D");
		m_rippleKernel = m_openCL.loadKernel("ripple2D");
		m_copyKernel = m_openCL.loadKernel("copy2D");
	}

	//--------------------------------------------------------------
	void CmbSceneCL2D::addDrop()
	{
		m_dropKernel->setArg(0, m_clImages[m_prevIdx]);
		m_dropKernel->setArg(1, ofVec2f(ofRandom(m_dimensions.x), ofRandom(m_dimensions.y)));
		m_dropKernel->setArg(2, m_radius);
		m_dropKernel->setArg(3, m_ringSize);
		m_dropKernel->setArg(4, m_dropColor);
		m_dropKernel->run2D(m_dimensions.x, m_dimensions.y);

		m_openCL.finish();
	}

	//--------------------------------------------------------------
	void CmbSceneCL2D::stepRipple()
	{
		m_rippleKernel->setArg(0, m_clImages[m_prevIdx]);
		m_rippleKernel->setArg(1, m_clImages[m_currIdx]);
		m_rippleKernel->setArg(2, m_clImages[m_tempIdx]);
		m_rippleKernel->setArg(3, m_damping / 10.0f + 0.9f);  // 0.9 - 1.0 range

		m_rippleKernel->run2D(m_dimensions.x, m_dimensions.y);

		m_openCL.finish();
	}

	//--------------------------------------------------------------
	void CmbSceneCL2D::copyResult()
	{
		m_copyKernel->setArg(0, m_clImages[m_tempIdx]);
		m_copyKernel->setArg(1, m_clImages[m_currIdx]);

		m_copyKernel->run2D(m_dimensions.x, m_dimensions.y);

		m_openCL.finish();
	}

	//--------------------------------------------------------------
	void CmbSceneCL2D::draw()
	{
		m_clImages[m_prevIdx].draw(0, 0);
	}
}

#endif // COMPUTE_CL_2D
