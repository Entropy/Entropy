//
//  CellRenderer.h
//  Entropy
//
//  Created by Elias Zananiri on 2016-02-17.
//
//

#pragma once

#include "ofMain.h"
#include "ofxRange.h"

#include "SnapshotRamses.h"

namespace ent
{
    class CellRenderer
    {
    public:
        CellRenderer();
        ~CellRenderer();

        void setup(const std::string& folder);
		void clear();

        void update();
        void draw(float scale);

        bool imGui(ofDefaultVec2& windowPos, ofDefaultVec2& windowSize);

    protected:
		// Data
		std::vector<SnapshotRamses *> m_snapshots;
		std::size_t m_currIndex;
		
		ofxRange3f m_coordRange;
		ofxRange1f m_sizeRange;
		ofxRange1f m_densityRange;

        // 3D Render
        bool m_bRender;

        ofDefaultVec3 m_originShift;
        float m_normalizeFactor;

        ofShader m_renderShader;

        float m_densityMin;
        float m_densityMax;
    };
}
