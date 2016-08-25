#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4, 1);
	settings.width = 1920;
	settings.height = 1080;
	settings.multiMonitorFullScreen = true;
	settings.decorated = false;
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}
