#include "ofMain.h"
#include "CMBApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(2, 1);
    settings.width = 1024;
    settings.height = 768;
    ofCreateWindow(settings);

    ofRunApp(new entropy::CMBApp());
}
