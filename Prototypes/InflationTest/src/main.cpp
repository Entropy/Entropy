#include "ofMain.h"
#include "InflationApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
	settings.setGLVersion(3,3);
    settings.width = 1920;
    settings.height = 1080;
	ofCreateWindow(settings);
	return ofRunApp(new entropy::InflationApp);
}
