#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4, 5);
	settings.width = 1920;
	settings.height = 1080;
	settings.resizable = false;
	settings.multiMonitorFullScreen = true;
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}
