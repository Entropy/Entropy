#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTimeline.h"

#include "entropy/Helpers.h"
#include "entropy/bubbles/Constants.h"
#ifdef COMPUTE_GL_2D
	#include "entropy/bubbles/PoolGL2D.h"
#endif
#ifdef COMPUTE_GL_3D
	#include "entropy/bubbles/PoolGL3D.h"
#endif
#include "entropy/geom/Sphere.h"
#include "entropy/render/PostEffects.h"
#include "entropy/render/WireframeFillRenderer.h"

class ofApp 
	: public ofBaseApp
{
public:
	void setup();
	void exit();

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	void reset();

	bool loadPreset(const string & presetName);
	bool savePreset(const string & presetName);

protected:
#ifdef COMPUTE_GL_2D
	entropy::bubbles::PoolGL2D pool2D;
#endif
#ifdef COMPUTE_GL_2D
	entropy::bubbles::PoolGL3D pool3D;
#endif

	entropy::geom::Sphere sphereGeom;
	ofTexture sphereTexture;
	ofShader sphereShader;

	ofEasyCam camera;
	
	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	ofFbo fboScene;
	ofFbo fboPost;

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;
	ofxTLCameraTrack cameraTrack;

	struct : ofParameterGroup
	{
		struct : ofParameterGroup
		{
			ofParameter<ofFloatColor> tintColor{ "Tint Color", ofFloatColor::white };
			ofParameter<float> maskMix{ "Mask Mix", 1.0f, 0.0f, 1.0f };

			PARAM_DECLARE("SphereExtra", tintColor, maskMix);
		} sphere;

		PARAM_DECLARE("Bubbles", sphere);
	} parameters;

	std::string currPreset;
};
