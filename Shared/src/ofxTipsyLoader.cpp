//
//  ofxTipsyLoader.cpp
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-28.
//
//

#include "ofxTipsyLoader.h"

//--------------------------------------------------------------
struct HeaderInfo
{
    double time;
    int nbodies;
    int ndim;
    int nsph;
    int ndark;
    int nstar;
} ;

//--------------------------------------------------------------
struct GasParticle
{
    float mass;
    float pos[3];
    float vel[3];
    float rho;
    float temp;
    float hsmooth;
    float metals;
    float phi;
};

//--------------------------------------------------------------
struct DarkParticle
{
    float mass;
    float pos[3];
    float vel[3];
    float eps;
    int phi;
};

//--------------------------------------------------------------
struct StarParticle
{
    float mass;
    float pos[3];
    float vel[3];
    float metals;
    float tform;
    float eps;
    int phi;
};

//--------------------------------------------------------------
bool ofxLoadTipsyFile(const string& filename,
                      int& numTotal,
                      int& numDarkParticles,
                      int& numStarParticles,
                      vector<ofVec4f>& bodyPositions,
                      vector<ofVec4f>& bodyVelocities,
                      vector<int>& bodyIDs)
{
    ofLogNotice("ofxLoadTipsyFile", "Opening file " + filename);

    ifstream inputFile(ofToDataPath(filename), ios::in | ios::binary);
    if (!inputFile.is_open()) {
        ofLogError("ofxLoadTipsyFile", "Could not open file " + filename);
        return false;
    }

    // Read the header.
    HeaderInfo headerInfo;
    inputFile.read((char *)&headerInfo, sizeof(headerInfo));
    numTotal = headerInfo.nbodies;
    numDarkParticles = headerInfo.ndark;
    numStarParticles = headerInfo.nstar;
//    numGasParticles = headerInfo.nsph;

    // Read through the particles one by one.
    int currIndex;
    ofVec4f currPos;
    ofVec4f currVel;

    DarkParticle darkParticle;
    StarParticle starParticle;

    int particleCount = 0;
    for (int i=0; i < numTotal; ++i) {
        if (i < numDarkParticles) {
            inputFile.read((char *)&darkParticle, sizeof(darkParticle));

            currIndex = darkParticle.phi;

            currPos.x = darkParticle.pos[0];
            currPos.y = darkParticle.pos[1];
            currPos.z = darkParticle.pos[2];
            currPos.w = darkParticle.mass;

            currVel.x = darkParticle.vel[0];
            currVel.y = darkParticle.vel[1];
            currVel.z = darkParticle.vel[2];
            currVel.w = darkParticle.eps;
        }
        else {
            inputFile.read((char *)&starParticle, sizeof(starParticle));

            currIndex = starParticle.phi;

            currPos.x = starParticle.pos[0];
            currPos.y = starParticle.pos[1];
            currPos.z = starParticle.pos[2];
            currPos.w = starParticle.mass;

            currVel.x = starParticle.vel[0];
            currVel.y = starParticle.vel[1];
            currVel.z = starParticle.vel[2];
            currVel.w = starParticle.eps;
        }

        bodyPositions.push_back(currPos);
        bodyVelocities.push_back(currVel);
        bodyIDs.push_back(currIndex);

        ++particleCount;
    }

    // Round up to a multiple of 256 bodies since our kernel only supports that.
    int roundTotal = numTotal;

    if (numTotal % 256) {
        roundTotal = ((numTotal / 256) + 1) * 256;
    }

    for (int i = numTotal; i < roundTotal; ++i) {
        currPos.set(0.0);
        currVel.set(0.0);

        bodyPositions.push_back(currPos);
        bodyVelocities.push_back(currVel);
        bodyIDs.push_back(i);

        ++numDarkParticles;
    }

    numTotal = roundTotal;

    inputFile.close();

    ofLogNotice("ofxLoadTipsyFile", "Read %d bodies", numTotal);
}
