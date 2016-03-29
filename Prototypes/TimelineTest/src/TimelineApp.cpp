#include "TimelineApp.h"

namespace entropy
{
    //--------------------------------------------------------------
    void TimelineEpoch::setup(int start, int end, string name)
    {
        timeStart = start;
        timeEnd = end;
        epochName = name;
    }

    //--------------------------------------------------------------
    void TimelineApp::setup()
    {
        timeStart = -430;
        timeEnd = 150;

        xLeft = 0;
        xRight = ofGetWidth();
        xLength = xRight - xLeft;

        tickSliceBig = 10;
        tickSliceSmall = 2;
        tickHeightBig = 10.0;
        tickHeightSmall = 5.0;

        linearTime[-120] = "one picosecond";
        linearTime[-90] = "one nanosecond";
        linearTime[-60] = "one microsecond";
        linearTime[-30] = "one millisecond";
        linearTime[0] = "one second";
        linearTime[35] = "one hour";
        linearTime[75] = "one year";
        linearTime[105] = "one thousand years";
        linearTime[135] = "one million years";

        epochs.resize(6);
        epochs[0].setup(-430, -360, "grand unification epoch");
        epochs[1].setup(-360, -120, "electroweak epoch");
        epochs[2].setup(-120, -60, "quark epoch");
        epochs[3].setup(-60, 0, "hadron epoch");
        epochs[4].setup(0, 25, "lepton epoch");
        epochs[5].setup(25, 130, "photon epoch");

        currEpoch = -1;
        keyPressed(OF_KEY_RIGHT);

        tickFont.load("GothamNarrow-Book.otf", 12);
    }

    //--------------------------------------------------------------
    void TimelineApp::update()
    {
        float ratio = 0.2;
        currStart = ofLerp(currStart, targetStart, ratio);
        currEnd = ofLerp(currEnd, targetEnd, ratio);
    }

    //--------------------------------------------------------------
    void TimelineApp::draw()
    {
        ofBackground(0);

        float top = ofGetHeight() - 80;
//        float top = 0;

        // Draw the linear time, if in range.
        float timeY = top - 20;
        for (auto& it : linearTime) {
            int time = it.first;
            if (epochs[currEpoch].timeStart < time && time <= epochs[currEpoch].timeEnd) {
                float timeX = ofMap(time, timeStart, timeEnd, xLeft, xRight);
                ofDrawLine(timeX, top, timeX, timeY);
                tickFont.drawString(it.second, timeX, timeY);
                timeY -= 20;
            }
        }

        // Draw the epoch.
        ofSetColor(200, 0, 0, 128);
        ofDrawRectangle(currStart, top, currEnd - currStart, ofGetHeight() - top);

        ofSetColor(200, 0, 0);
        float labelX = currStart;
        float labelY = top - 200;
        ofDrawLine(currStart, top, labelX, labelY);
        tickFont.drawString(epochs[currEpoch].epochName, labelX, labelY);

        ofSetColor(255);

        // Draw the timeline.
        ofDrawLine(xLeft, top, xRight, top);

        // Add the ticks.
        int numTicks = (timeEnd - timeStart) / tickSliceSmall;
        int periodBig = tickSliceBig / tickSliceSmall;
        float pxPerTick = xLength / (float)numTicks;
        float x = xLeft;
        float y = top;
        for (int i = 0; i < numTicks; ++i) {
            if (i % periodBig == 0) {
                ofDrawLine(x, y, x, y + tickHeightBig);
            }
            else {
                ofDrawLine(x, y, x, y + tickHeightSmall);
            }
            x += pxPerTick;
        }

        // Add the labels.
        int numLabels = (timeEnd - timeStart) / tickSliceBig;
        float pxPerLabel = xLength / float(numLabels);
        int t = timeStart;
        x = xLeft;
        y = top + tickHeightBig;

        ofPushMatrix();
        ofTranslate(xLeft + tickFont.stringHeight("-0000") / 2, y + tickFont.stringWidth("-0000"));
        ofRotate(-90, 0, 0, 1);
        for (int i = 0; i < numLabels; ++i) {
            tickFont.drawString(ofToString(t), 0, x);
            x += pxPerLabel;
            t += tickSliceBig;
        }
        ofPopMatrix();
    }

    //--------------------------------------------------------------
    void TimelineApp::keyPressed(int key)
    {
        if (key == OF_KEY_RIGHT) {
            currEpoch = MIN(currEpoch + 1, epochs.size() - 1);
        }
        else if (key == OF_KEY_LEFT) {
            currEpoch = MAX(currEpoch - 1, 0);
        }
        targetStart = ofMap(epochs[currEpoch].timeStart, timeStart, timeEnd, xLeft, xRight);
        targetEnd = ofMap(epochs[currEpoch].timeEnd, timeStart, timeEnd, xLeft, xRight);
    }

    //--------------------------------------------------------------
    void TimelineApp::keyReleased(int key){

    }

    //--------------------------------------------------------------
    void TimelineApp::mouseMoved(int x, int y ){

    }

    //--------------------------------------------------------------
    void TimelineApp::mouseDragged(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void TimelineApp::mousePressed(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void TimelineApp::mouseReleased(int x, int y, int button){

    }

    //--------------------------------------------------------------
    void TimelineApp::mouseEntered(int x, int y){

    }

    //--------------------------------------------------------------
    void TimelineApp::mouseExited(int x, int y){

    }

    //--------------------------------------------------------------
    void TimelineApp::windowResized(int w, int h){
        
    }
    
    //--------------------------------------------------------------
    void TimelineApp::gotMessage(ofMessage msg){
        
    }
    
    //--------------------------------------------------------------
    void TimelineApp::dragEvent(ofDragInfo dragInfo){ 
        
    }
}
