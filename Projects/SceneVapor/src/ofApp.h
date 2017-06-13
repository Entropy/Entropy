#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxTimeline.h"

#include "SequenceRamses.h"
#include "ofxTextureRecorder.h"
#include "WireframeFillRenderer.h"
#include "PostEffects.h"
#include "EagleOctree.h"

class ofApp 
	: public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
	void reloadOctree();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    ent::SequenceRamses m_sequenceRamses;

    ofEasyCam m_camera;

	ofParameter<float> m_scale{"world scale", 1, 0.1, 100};
	ofParameter<float> m_fov{"camera fov", 60, 0.001, 100, ofParameterScale::Logarithmic};
	ofParameter<float> m_octreeAnimationDuration{"octree anim duration", 60, 1, 15000, ofParameterScale::Logarithmic};
	ofParameter<float> m_orbitStart{"camera orbit start", 15, 0, 600, ofParameterScale::Logarithmic};
	ofParameter<float> m_orbitEnd{"camera orbit end", 45, 0, 600, ofParameterScale::Logarithmic};
	ofParameter<float> m_finalRadius{"camera final radius", 3, 0, 10, ofParameterScale::Logarithmic};
	ofParameter<float> m_longitude{"camera orbit longitude", 0, 0, 360};
	ofParameter<float> m_latitude{"camera orbit latitude", 0, 0, 360};
	ofParameter<float> m_alphaInitial{"octree anim alpha", 10, 0, 10};
	ofParameter<float> m_alphaFilter{"octree alpha filter", 0.999, 0.9, 0.99};
	ofParameter<bool> m_cameraAutoDistance{"camera auto distance", false};
	ofParameter<float> m_cameraAutoDistanceMagnification{"camera auto magnification", 1, 0.00001, 100, ofParameterScale::Logarithmic};
	ofParameter<float> m_cameraCenterToEagle{"center to eagle factor", 0, 0, 1};
	ofParameter<bool> m_playbackEagle{"playback eagle", false};
	ofParameter<bool> m_glDebug{"debug gl", false};
	ofParameter<bool> m_showOctree{"show octree", false};
	ofParameter<bool> m_showOctreeH{"show octree h", false};
	ofParameter<bool> m_showOctreeV{"show octree v", false};
	ofParameter<bool> m_showOctreeD{"show octree d", false};
	ofParameter<bool> m_showOctreeDensities{"show octree densities", false};
	ofParameter<bool> m_showAxis{"show axis", false};
	ofParameter<bool> m_showOctreeAnimation{"show octree animation", false};
	ofParameter<bool> m_showOctreeAnimationH{"show octree animation h", false};
	ofParameter<bool> m_showOctreeAnimationV{"show octree animation v", false};
	ofParameter<bool> m_showOctreeAnimationD{"show octree animation d", false};
	ofParameter<float> m_offsetOctreeH{"offset octree H", 0, -2400, 2400};
	ofParameter<float> m_offsetOctreeV{"offset octree V", 0, -2400, 2400};
	ofParameter<float> m_offsetOctreeD{"offset octree D", 0, -2400, 2400};
	ofParameter<bool> m_showEagleOctree{"show eagle octree", false};
	ofParameter<float> m_eagleOctreeAlpha{"eagle octree alpha", 0, 0, 1};
	ofParameter<float> m_eagleMincubeAlpha{"eagle mincube alpha", 0, 0, 1};
	ofParameter<glm::vec3> m_eagleTrackOffset{"eagle track offset", glm::vec3(0), glm::vec3(-20000,-1000,-1000), glm::vec3(20000,1000,1000)};
	ofParameter<float> m_eagleTrackRotationCenterX{"eagle track rotation center", 0,-20000,20000};
	ofParameter<float> m_eagleTrackRotation{"eagle track rotation", 0, -180, 180};
	ofParameter<ofFloatColor> m_octreeColor{"octree color", ofFloatColor::white};
	ofParameter<float> m_octreeAlpha{"octree alpha", 1, 0, 1};
	ofParameter<bool> m_vboTex{"vbo texture", false};
	ofParameter<bool> m_bSyncPlayback{"sync playback", false};
	ofParameter<bool> m_bExportFrames{"record", false};
	ofParameter<bool> m_bRecordVideo{"record video", false};
	ofParameter<bool> m_bShowTimeline{"show timeline", true};
	ofParameter<string> m_exportPath{ofToDataPath("",true)};
	ofParameter<bool> autoMode{"auto mode", false};
	ofParameter<bool> frameInterpolation{"frame interpolation", false};
	ofParameter<int> animationFps{"animation fps", 6, 0, 60};
	ofParameter<float> eagleVideoFps{"eagle fps", 12, 0, 30};
	ofParameter<float> eagleVideoAlpha{"eagle video alpha", 0, 0, 1};
	ofParameterGroup appParameters{
		"application",
		m_scale,
		m_fov,
		m_glDebug,
		m_showOctree,
		m_showOctreeH,
		m_showOctreeV,
		m_showOctreeD,
		m_showOctreeDensities,
		m_showOctreeAnimation,
		m_showOctreeAnimationH,
		m_offsetOctreeH,
		m_showOctreeAnimationV,
		m_offsetOctreeV,
		m_showOctreeAnimationD,
		m_offsetOctreeD,
		m_showEagleOctree,
		m_playbackEagle,
		m_octreeColor,
		m_octreeAlpha,
		m_octreeAnimationDuration,
		m_alphaInitial,
		m_alphaFilter,
		m_showAxis,
		m_orbitStart,
		m_orbitEnd,
		m_finalRadius,
		m_longitude,
		m_latitude,
		m_cameraAutoDistance,
		m_cameraAutoDistanceMagnification,
		m_cameraCenterToEagle,
		m_eagleOctreeAlpha,
		m_eagleMincubeAlpha,
		m_eagleTrackOffset,
		m_eagleTrackRotationCenterX,
		m_eagleTrackRotation,
		eagleVideoFps,
		eagleVideoAlpha,
		animationFps,
		//m_vboTex,
		m_bSyncPlayback,
		m_bExportFrames,
		m_bRecordVideo,
		m_bShowTimeline,
		m_exportPath,
		autoMode,
				frameInterpolation,
	};


	ofParameterGroup parameters{
		"vapor",
		appParameters,
		m_sequenceRamses.parameters,
	};

	ofxTimeline m_timeline;
	//ofxTLCameraTrack *m_cameraTrack;

	ofxPanel m_gui;
	bool m_bGuiVisible = true;
	ofxTextureRecorder recorder;

	ofTrueTypeFont ttf;
	double octreeAnimationStart;
	float octreeTotalDistanceH, octreeTotalDistanceV, octreeTotalDistanceD;
	std::vector<ofEventListener> listeners;
	ofVbo octreeAnimationVboH, octreeAnimationVboV;
	ofMesh octreeAnimationMeshH, octreeAnimationMeshV, octreeAnimationMeshD;

	struct Range{
		Range(double startTime,
				ofIndexType startIndex,
				ofIndexType endIndex)
			:startTime(startTime)
			,index(startIndex)
			,startIndex(startIndex)
			,endIndex(endIndex){
			lines.setMode(OF_PRIMITIVE_LINES);
		}

		double startTime;
		ofIndexType index;
		ofIndexType startIndex;
		ofIndexType endIndex;
		ofMesh lines;
		std::vector<size_t> linesIndex;
	};

	std::vector<Range> rangesH;
	std::vector<Range> rangesV;
	std::vector<Range> rangesD;

	float orbitStartDistance = 0;

	ofShader shader;
	ofShader frameInterpolationShader;
	ofFbo fbo, fboLines, fboPost, fboPostLines, fboComposite;
	double vaporAnimationTime, vaporAnimationLoopStart = 0;
	double vaporAnimationLoopEnd = 1;

	entropy::render::WireframeFillRenderer renderer;

	EagleOctree eagleOctree;
	double eagleTime = 0;
	double eagleFrame = 0;
	double now = 0;
	double dt = 0;


	ofImage eagleImg1, eagleImg2;

	entropy::render::PostEffects posteffects;
	entropy::render::PostParameters postParameters, linesPostParameters;
};
