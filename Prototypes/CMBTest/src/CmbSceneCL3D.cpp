#include "CmbSceneCL3D.h"

namespace ent
{
	//--------------------------------------------------------------
	void CmbSceneCL3D::setup()
	{
		CmbScene::setup();

		m_openCL.setupFromOpenGL();

		// Allocate the textures and buffers.
		for (int i = 0; i < 3; ++i) {
			m_clImages[i].initWithTexture3D(m_dimensions.x, m_dimensions.y, m_dimensions.z, GL_RGBA16F);
		}

		// Load program and kernels.
		m_openCL.loadProgramFromFile("cl/cmb.cl");

		m_dropKernel = m_openCL.loadKernel("drop3D");
		m_rippleKernel = m_openCL.loadKernel("ripple3D");
		m_copyKernel = m_openCL.loadKernel("copy3D");
	}

	//--------------------------------------------------------------
	void CmbSceneCL3D::addDrop()
	{
		m_dropKernel->setArg(0, m_clImages[m_prevIdx]);
		m_dropKernel->setArg(1, ofVec4f(ofRandom(m_dimensions.x), ofRandom(m_dimensions.y), ofRandom(m_dimensions.z), 0));
		m_dropKernel->setArg(2, m_radius);
		m_dropKernel->setArg(3, m_ringSize);
		m_dropKernel->setArg(4, m_dropColor);
		m_dropKernel->run3D(m_dimensions.x, m_dimensions.y, m_dimensions.z);

		m_openCL.finish();
	}

	//--------------------------------------------------------------
	void CmbSceneCL3D::stepRipple()
	{
		m_rippleKernel->setArg(0, m_clImages[m_prevIdx]);
		m_rippleKernel->setArg(1, m_clImages[m_currIdx]);
		m_rippleKernel->setArg(2, m_clImages[m_tempIdx]);
		m_rippleKernel->setArg(3, m_damping / 10.0f + 0.9f);  // 0.9 - 1.0 range

		m_rippleKernel->run3D(m_dimensions.x, m_dimensions.y, m_dimensions.z);

		m_openCL.finish();
	}

	//--------------------------------------------------------------
	void CmbSceneCL3D::copyResult()
	{
		m_copyKernel->setArg(0, m_clImages[m_tempIdx]);
		m_copyKernel->setArg(1, m_clImages[m_currIdx]);

		m_copyKernel->run3D(m_dimensions.x, m_dimensions.y, m_dimensions.z);

		m_openCL.finish();

		m_volumetrics.setup(&m_clImages[m_currIdx].getTexture3D(), ofVec3f(1, 1, 1));
	}

	//--------------------------------------------------------------
	void CmbSceneCL3D::draw()
	{
		m_volumetrics.setRenderSettings(1.0, 1.0, 1.0, 0.1);
		m_volumetrics.drawVolume(0, 0, 0, ofGetHeight(), 0);
	}
}
