#include "CmbScene.h"

namespace ent
{
	//--------------------------------------------------------------
	CmbScene::CmbScene()
		: m_bRestart(true)
		, m_dropColor(ofColor::red)
		, m_bDropping(true)
		, m_dropRate(1)
		, m_damping(0.995f)
		, m_radius(30.0f)
		, m_ringSize(0.5f)
	{}

	//--------------------------------------------------------------
	void CmbScene::setup()
	{
		m_prevIdx = 0;
		m_currIdx = 1;
		m_tempIdx = 2;

		m_bRestart = true;
	}

	//--------------------------------------------------------------
	void CmbScene::update()
	{
		if (m_bRestart)
		{
			setup();

			m_bRestart = false;
		}

		if (m_bDropping && ofGetFrameNum() % m_dropRate == 0)
		{
			addDrop();
		}

		stepRipple();
		copyResult();

		std::swap(m_currIdx, m_prevIdx);
	}

	//--------------------------------------------------------------
	void CmbScene::setDimensions(int size)
	{
		setDimensions(glm::vec3(size));
	}

	//--------------------------------------------------------------
	void CmbScene::setDimensions(const glm::vec2& dimensions)
	{
		setDimensions(glm::vec3(dimensions, 0.0f));
	}

	//--------------------------------------------------------------
	void CmbScene::setDimensions(const glm::vec3& dimensions)
	{
		m_dimensions = dimensions;

		m_bRestart = true;
	}
}
