#pragma once

#include "MSAOpenCL.h"

#include "CmbScene.h"

namespace ent
{
	class CmbSceneCL2D
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

		msa::OpenCLImage m_clImages[3];
	};
}
