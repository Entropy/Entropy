#pragma once

#include "MSAOpenCL.h"
#include "ofxVolumetrics3D.h"

#include "CmbScene.h"
#include "OpenCLImage3D.h"

namespace ent
{
	class CmbSceneCL3D
		: public CmbScene
	{
	public:
		void setup() override;
		void draw() override;

	protected:
		void addDrop() override;
		void stepRipple() override;
		void copyResult() override;

		msa::OpenCL m_openCL;
		msa::OpenCLKernelPtr m_dropKernel;
		msa::OpenCLKernelPtr m_rippleKernel;
		msa::OpenCLKernelPtr m_copyKernel;

		OpenCLImage3D m_clImages[3];
		ofxVolumetrics3D m_volumetrics;
	};
}