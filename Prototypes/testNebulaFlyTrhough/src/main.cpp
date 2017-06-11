#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main(){
	  ofGLFWWindowSettings settings;
	  settings.setGLVersion(3,3);
	  ofCreateWindow(settings);
	  ofRunApp(new ofApp);
}
