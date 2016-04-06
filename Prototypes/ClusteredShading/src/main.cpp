#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4,5);
    settings.width = 1920 * 2;
    settings.height = 1080;
    settings.multiMonitorFullScreen = true;
    settings.windowMode = OF_FULLSCREEN;
	ofCreateWindow(settings);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
