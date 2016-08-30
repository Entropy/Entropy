#pragma once

#ifdef _WIN32

#include "ofxTimeline.h"
#include "ofxWMFVideoPlayer.h"

class ofxTLWMFVideoTrack
	: public ofxTLTrack
{
public:
	ofxTLWMFVideoTrack();
	virtual ~ofxTLWMFVideoTrack();

	virtual void setup();
	virtual void draw();
	virtual void enable();
	virtual void disable();

	bool load(string moviePath);

	void setPlayer(ofxWMFVideoPlayer& newPlayer);
	void setPlayer(std::shared_ptr<ofxWMFVideoPlayer> newPlayer);
	std::shared_ptr<ofxWMFVideoPlayer> getPlayer();

	virtual void update();
	//	virtual void update(ofEventArgs& args);
	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);

	virtual void keyPressed(ofKeyEventArgs& args);

	virtual void playbackStarted(ofxTLPlaybackEventArgs& args);
	virtual void playbackLooped(ofxTLPlaybackEventArgs& args);
	virtual void playbackEnded(ofxTLPlaybackEventArgs& args);

	int getSelectedFrame();
	int getCurrentFrame();
	float getCurrentTime();
	float getDuration();

	void setDrawVideoPreview(bool drawPreview);
	bool getDrawVideoPreview();

	bool isLoaded();
	int selectFrame(int frame); //returns the true selected frame in video

	//Let's the video track have
	void setInFrame(int inFrame);
	void setOutFrame(int outFrame);

	virtual bool togglePlay();
	virtual void play();
	virtual void stop();
	virtual bool getIsPlaying();

	//plays when timeline is played
	void setPlayAlongToTimeline(bool playAlong);
	bool getPlayAlongToTimeline();

	virtual string getTrackType();

protected:
	float positionForSecond(float second);

	int selectedFrame;
	int currentLoop;
	bool isSetup;

	int inFrame;
	int outFrame;

	bool drawVideoPreview;
	bool currentlyPlaying;

	bool playAlongToTimeline;

	//width and height of image elements
	float getContentWidth();
	float getContentHeight();

	std::shared_ptr<ofxWMFVideoPlayer> player;

	void playheadScrubbed(ofxTLPlaybackEventArgs& args);
};

#endif

