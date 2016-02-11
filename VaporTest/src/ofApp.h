#pragma once

#include "ofMain.h"
#include "ofxHDF5.h"
#include "ofxImGui.h"
#include "ofxRange.h"

enum ExtraAttributes
{
    CELLSIZE_ATTRIBUTE = 5,
    DENSITY_ATTRIBUTE = 6
};

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // GUI
    void imGui();

    ofxImGui gui;
    bool bGuiVisible;
    bool bMouseOverGui;

    // Data
    void loadDataSet(const string& filename, vector<float>& data, int stride, bool bExponential);

    ofVboMesh vboMesh;
    vector<float> cellSize;
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

    ofShader renderShader;
    ofEasyCam cam;

    float pointSize;
    float densityMin;
    float densityMax;
    float scale;

    // Texture Output
    void rebuildBins();
    bool bNeedsBins;

    bool bCycleBins;
    bool bExportFiles;

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
