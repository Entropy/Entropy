#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTextureRecorder.h"
#include "ofxTimeline.h"

#include "entropy/Helpers.h"
#include "entropy/bubbles/Constants.h"
#ifdef COMPUTE_GL_2D
	#include "entropy/bubbles/PoolGL2D.h"
#endif
#ifdef COMPUTE_GL_3D
	#include "entropy/bubbles/PoolGL3D.h"
#endif
#include "entropy/geom/Box.h"
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

	glm::mat4 getWorldTransform() const;

	bool loadPreset(const string & presetName);
	bool savePreset(const string & presetName);

protected:
#ifdef COMPUTE_GL_2D
	entropy::bubbles::PoolGL2D pool2D;
#endif
#ifdef COMPUTE_GL_2D
	entropy::bubbles::PoolGL3D pool3D;
#endif

	entropy::geom::Box boxGeom;

	entropy::geom::Sphere sphereGeom;
	ofTexture sphereTexture;
	ofShader sphereShader;

	ofEasyCam camera;
	glm::vec3 tumbleOffset;
	float dollyOffset;
	
	entropy::render::WireframeFillRenderer renderer;
	entropy::render::PostEffects postEffects;
	entropy::render::PostParameters postParams;
	ofFbo fboScene;
	ofFbo fboPost;

	vector<ofEventListener> eventListeners;
	ofxPanel gui;
	ofxTimeline timeline;
	ofxTLCameraTrack cameraTrack;

	ofxTextureRecorder textureRecorder;

	std::string currPreset;

	struct : ofParameterGroup
	{
		struct : ofParameterGroup
		{
			ofParameter<bool> recordSequence{ "Record Sequence", false };
			ofParameter<bool> recordVideo{ "Record Video", false };

			PARAM_DECLARE("Render",
				recordSequence,
				recordVideo);
		} render;
		
		struct : ofParameterGroup
		{
			ofParameter<float> nearClip{ "Near Clip", 0.001f, 0.001f, 1000.0f, ofParameterScale::Logarithmic };
			ofParameter<float> farClip{ "Far Clip", 1000.0f, 1.0f, 200000.0f };
			ofParameter<float> tiltSpeed{ "Tilt Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> panSpeed{ "Pan Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> rollSpeed{ "Roll Speed", 0.0f, -1.0f, 1.0f };
			ofParameter<float> dollySpeed{ "Dolly Speed", 0.0f, -10.0f, 10.0f };

			PARAM_DECLARE("Camera",
				nearClip, farClip,
				tiltSpeed, panSpeed, rollSpeed,
				dollySpeed);
		} camera;

		struct : ofParameterGroup
		{
			ofParameter<ofFloatColor> tintColor{ "Tint Color", ofFloatColor::white };
			ofParameter<float> maskMix{ "Mask Mix", 1.0f, 0.0f, 1.0f };

			PARAM_DECLARE("SphereExtra", tintColor, maskMix);
		} sphere;

		PARAM_DECLARE("Bubbles",
			render,
			camera,
			sphere);
	} parameters;
};
