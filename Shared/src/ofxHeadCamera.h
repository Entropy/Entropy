//
//  ofxHeadCamera.hpp
//  ExpansionTest
//
//  Created by Elias Zananiri on 2016-01-25.
//
//

#pragma once

#include "ofMain.h"

class ofxHeadCamera : public ofCamera
{
public:
    void setup();
    void exit();

    void reset();

protected:
    void update(ofEventArgs& args);
    void mouseReleased(ofMouseEventArgs& args);

    float sensitivity;
    bool bMouseDown;
    unsigned long prevMillis;
    ofVec2f prevMouse;
    ofVec2f currMouse;
};
