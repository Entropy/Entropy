#include "ofMain.h"
#include "PartyCLApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(3, 2);
    settings.width = 1920;
    settings.height = 1080;
    ofCreateWindow(settings);

    ofRunApp(new entropy::PartyCLApp());
}
