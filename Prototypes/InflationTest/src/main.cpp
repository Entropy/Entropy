#include "ofMain.h"
#include "InflationApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 1);
    settings.width = 1920;
    settings.height = 1080;
    ofCreateWindow(settings);

    ofRunApp(new entropy::InflationApp());
}
