#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(3, 2);
    settings.width = 1280;
    settings.height = 1280;
    ofCreateWindow(settings);

    ofRunApp(new ofApp());
}
