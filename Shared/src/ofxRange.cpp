//
//  ofxRange.cpp
//  VaporTest
//
//  Created by Elias Zananiri on 2016-02-10.
//
//

#include "ofxRange.h"

//--------------------------------------------------------------
ofxFloatRange::ofxFloatRange()
: _min(FLT_MAX)
, _max(FLT_MIN)
{}

//--------------------------------------------------------------
ofxFloatRange::~ofxFloatRange()
{}

//--------------------------------------------------------------
void ofxFloatRange::set(float min, float max)
{
    _min = min;
    _max = max;
}

//--------------------------------------------------------------
void ofxFloatRange::add(float f)
{
    _min = min(_min, f);
    _max = max(_max, f);
}

//--------------------------------------------------------------
bool ofxFloatRange::contains(float f)
{
    return (_min <= f && f <= _max);
}

//--------------------------------------------------------------
float ofxFloatRange::getMin()
{
    return _min;
}

//--------------------------------------------------------------
float ofxFloatRange::getMax()
{
    return _max;
}

//--------------------------------------------------------------
float ofxFloatRange::getSpan()
{
    return (_max - _min);
}

//--------------------------------------------------------------
ofxVec3fRange::ofxVec3fRange()
: _min(FLT_MAX)
, _max(FLT_MIN)
{}

//--------------------------------------------------------------
ofxVec3fRange::~ofxVec3fRange()
{}

//--------------------------------------------------------------
void ofxVec3fRange::set(const ofVec3f& min, const ofVec3f& max)
{
    _min = min;
    _max = max;
}

//--------------------------------------------------------------
void ofxVec3fRange::add(const ofVec3f& v)
{
    _min.x = min(_min.x, v.x);
    _min.y = min(_min.y, v.y);
    _min.z = min(_min.z, v.z);

    _max.x = max(_max.x, v.x);
    _max.y = max(_max.y, v.y);
    _max.z = max(_max.z, v.z);
}

//--------------------------------------------------------------
bool ofxVec3fRange::contains(const ofVec3f& v)
{
    return (_min.x <= v.x && v.x <= _max.x &&
            _min.y <= v.y && v.y <= _max.y &&
            _min.z <= v.z && v.z <= _max.z);
}

//--------------------------------------------------------------
const ofVec3f& ofxVec3fRange::getMin()
{
    return _min;
}

//--------------------------------------------------------------
const ofVec3f& ofxVec3fRange::getMax()
{
    return _max;
}

//--------------------------------------------------------------
ofVec3f ofxVec3fRange::getSpan()
{
    return (_max - _min);
}
