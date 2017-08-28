#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4, 5);
    settings.width = 1920;
    settings.height = 1080;
    ofCreateWindow(settings);

    ofRunApp(new ofApp());
}
