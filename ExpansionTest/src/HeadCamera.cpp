//
//  HeadCamera.cpp
//  ExpansionTest
//
//  Created by Elias Zananiri on 2016-01-25.
//
//

#include "HeadCamera.h"

namespace easy
{
    //--------------------------------------------------------------
    void HeadCamera::setup()
    {
        setNearClip(0.0);
        setFarClip(FLT_MAX);

        sensitivity = 1.0;
        prevMillis = 0;
        bMouseDown = false;

        reset();

        ofAddListener(ofEvents().update, this, &HeadCamera::update);
        ofAddListener(ofEvents().mouseReleased, this, &HeadCamera::mouseReleased);
    }

    //--------------------------------------------------------------
    void HeadCamera::exit()
    {
        ofRemoveListener(ofEvents().update, this, &HeadCamera::update);
        ofRemoveListener(ofEvents().mouseReleased, this, &HeadCamera::mouseReleased);
    }

    //--------------------------------------------------------------
    void HeadCamera::reset()
    {
        lookAt(getLookAtDir());
    }

    //--------------------------------------------------------------
    void HeadCamera::update(ofEventArgs& args)
    {
        if (ofGetMousePressed(0)) {
            if (!bMouseDown) {
                bMouseDown = true;

                prevMouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
            }
            else {
                prevMouse = currMouse;
            }

            currMouse = ofVec2f(ofGetMouseX(), ofGetMouseY());

            ofVec2f offset = currMouse - prevMouse;
            float factor = sensitivity * 180.0 / min(ofGetViewportWidth(), ofGetViewportHeight());

            ofVec3f rotation = ofVec3f(-offset.y * factor,
                                       -offset.x * factor,
                                       0.0);

            ofQuaternion orientation = ofQuaternion(rotation.x, getXAxis(), rotation.y, getYAxis(), rotation.z, getZAxis());
            setOrientation(ofCamera::getGlobalOrientation() * orientation);
        }
        else if (bMouseDown) {
            bMouseDown = false;
        }
    }

    //--------------------------------------------------------------
    void HeadCamera::mouseReleased(ofMouseEventArgs& args)
    {
        static const unsigned long kDoubleClickTime = 200;

        unsigned long currMillis = ofGetElapsedTimeMillis();
        if (prevMillis != 0 && currMillis - prevMillis < kDoubleClickTime) {
            reset();
            return;
        }
        prevMillis = currMillis;
    }
}