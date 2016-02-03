//
//  ofxTipsyLoader.h
//  PartyCL
//
//  Created by Elias Zananiri on 2016-01-28.
//
//

#pragma once

#include "ofMain.h"

/* 
 * Custom version of the tipsy file format written by Jeroen Bedorf.
 * Most important change is that we store particle ID on the location 
 * where previously the potential was stored. 
 */

//--------------------------------------------------------------
bool ofxLoadTipsyFile(const string& filename,
                      int& numTotal,
                      int& numDarkParticles,
                      int& numStarParticles,
                      vector<ofVec4f>& bodyPositions,
                      vector<ofVec4f>& bodyVelocities,
                      vector<int>& bodyIDs);
