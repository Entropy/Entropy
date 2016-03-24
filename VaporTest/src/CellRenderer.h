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

namespace ent
{
    enum ExtraAttributes
    {
        DENSITY_ATTRIBUTE = 5
    };

    class CellRenderer
    {
    public:
        CellRenderer();
        ~CellRenderer();

        void setup();

        void update();
        void draw(float scale);

        bool imGui(ofVec2f& windowPos, ofVec2f& windowSize);

    protected:
        // Data
        void loadDataSet(const string& filename, vector<float>& data, int stride, bool bExponential);
        ofVboMesh vboMesh;
        vector<ofMatrix4x4> matrices;
        vector<float> density;

        ofxVec3fRange coordRange;
        ofxFloatRange cellSizeRange;
        ofxFloatRange densityRange;

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
