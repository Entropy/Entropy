#include "ofMain.h"
#include "ofApp.h"

//#define TWO_1080P_SCREENS

//========================================================================
int main()
{
	ofGLFWWindowSettings settings;
	settings.setGLVersion(4,5);
    //settings.windowMode = OF_FULLSCREEN;
    settings.width = 1920;
    settings.height = 1080;
    settings.depthBits = 16;

#ifdef TWO_1080P_SCREENS
    settings.width = 1920 * 2;
    settings.multiMonitorFullScreen = true;
#endif

	ofCreateWindow( settings );

	ofRunApp( new ofApp() );
}
