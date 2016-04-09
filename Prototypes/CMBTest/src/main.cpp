#include "ofApp.h"

//========================================================================
int main()
{
    ofGLWindowSettings settings;
#ifdef COMPUTE_GL_3D
    settings.setGLVersion(4, 3);
#else
    settings.setGLVersion(4, 1);
#endif
    settings.width = 1920;
    settings.height = 1080;
    ofCreateWindow(settings);

    ofRunApp(new ent::ofApp());
}
