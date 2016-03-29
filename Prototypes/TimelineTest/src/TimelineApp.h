#pragma once

#include "ofMain.h"

namespace entropy
{
    class TimelineEpoch
    {
    public:
        void setup(int start, int end, string name);

        int timeStart;
        int timeEnd;
        string epochName;
    };

    class TimelineApp : public ofBaseApp
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

        int timeStart;
        int timeEnd;

        int tickSliceBig;
        int tickSliceSmall;
        float tickHeightBig;
        float tickHeightSmall;

        map<int, string> linearTime;
        vector<TimelineEpoch> epochs;

        int currEpoch;

        float currStart;
        float currEnd;
        float targetStart;
        float targetEnd;

        float xLeft;
        float xRight;
        float xLength;

        ofTrueTypeFont tickFont;
    };
}
