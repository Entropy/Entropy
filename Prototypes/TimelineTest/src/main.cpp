#include "ofMain.h"
#include "TimelineApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(3, 2);
    settings.width = 1920;
    settings.height = 768;
    ofCreateWindow(settings);

    ofRunApp(new entropy::TimelineApp());
}
