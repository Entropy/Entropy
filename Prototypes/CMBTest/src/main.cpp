#include "ofApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
    settings.setGLVersion(4, 1);
    settings.width = 1024;
    settings.height = 768;
    ofCreateWindow(settings);

    ofRunApp(new ent::ofApp());
}
