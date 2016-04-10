#pragma once

#include "ofMain.h"

namespace ent
{
	class CmbScene
	{
	public:
		CmbScene();
		
		virtual void setup() = 0;
		virtual void update();
		virtual void draw() = 0;

		void setDimensions(int size);
		void setDimensions(const ofVec2f& dimensions);
		void setDimensions(const ofVec3f& dimensions);

		ofFloatColor m_dropColor;

		bool m_bDropping;
		int m_dropRate;

		float m_damping;
		float m_radius;
		float m_ringSize;

		bool m_bRestart;

	protected:
		virtual void addDrop() = 0;
		virtual void stepRipple() = 0;
		virtual void copyResult() = 0;

		ofVec3f m_dimensions;

		int m_currIdx;
		int m_prevIdx;
		int m_tempIdx;
	};
}