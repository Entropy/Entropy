#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main()
{
	ofGLWindowSettings settings;
	settings.setGLVersion(3, 2);
	settings.width = 640;
	settings.height = 480;
	ofCreateWindow(settings);

	ofRunApp(new ofApp());
}
