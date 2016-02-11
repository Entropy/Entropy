//
//  ofxRange.hpp
//  VaporTest
//
//  Created by Elias Zananiri on 2016-02-10.
//
//

#pragma once

#include "ofMain.h"

class ofxFloatRange
{
public:
    ofxFloatRange();
    ~ofxFloatRange();

    void set(float min, float max);
    void add(float f);
    bool contains(float f);

    float getMin();
    float getMax();
    float getSpan();

protected:
    float _min;
    float _max;
};

class ofxVec3fRange
{
public:
    ofxVec3fRange();
    ~ofxVec3fRange();

    void set(const ofVec3f& min, const ofVec3f& max);
    void add(const ofVec3f& f);
    bool contains(const ofVec3f& f);

    const ofVec3f& getMin();
    const ofVec3f& getMax();
    ofVec3f getSpan();

protected:
    ofVec3f _min;
    ofVec3f _max;
};
