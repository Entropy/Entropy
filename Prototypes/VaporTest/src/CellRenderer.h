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

        int stride;

        // 3D Render
        void rebuildIndices();
        bool bNeedsIndices;

        bool bRender3D;

        ofVec3f originShift;
        float normalizeFactor;

        ofBufferObject bufferObject;
        ofTexture bufferTexture;
        ofShader renderShader;

        float pointSize;
        float densityMin;
        float densityMax;

        // Texture Output
        void rebuildBins();
        bool bNeedsBins;

        bool bCycleBins;
        bool bExportFiles;
        string exportFolder;

        int binPower;
        int binSizeX, binSizeY, binSizeZ;
        float binSliceZ;
        
        int binIndex;
        int renderIndex;
        
        float pointAdjust;
        
        bool bBinDebug2D;
        bool bBinDebug3D;
        
        ofShader sliceShader;
        ofFbo binFbo;
        ofPixels binPixels;
    };
}
