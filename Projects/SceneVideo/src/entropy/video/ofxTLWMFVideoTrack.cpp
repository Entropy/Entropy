#ifdef _WIN32

#include "ofxTLWMFVideoTrack.h"
#include "ofxTimeline.h"

ofxTLWMFVideoTrack::ofxTLWMFVideoTrack()
	: ofxTLTrack()
	, player(nullptr)
	, currentLoop(0)
	, inFrame(-1)
	, outFrame(-1)
	, currentlyPlaying(false)
	, drawVideoPreview(true)
	, playAlongToTimeline(true)
	, isSetup(false)
{}

ofxTLWMFVideoTrack::~ofxTLWMFVideoTrack() 
{
	if (isSetup) 
	{
		disable();
	}
}

void ofxTLWMFVideoTrack::setup() 
{
	ofxTLTrack::setup();
	isSetup = true;
}

void ofxTLWMFVideoTrack::enable() 
{
	if (!enabled) 
	{
		ofxTLTrack::enable();
		ofAddListener(events().playheadScrubbed, this, &ofxTLWMFVideoTrack::playheadScrubbed);
	}
}

void ofxTLWMFVideoTrack::disable() 
{
	if (enabled) 
	{
		stop();
		ofxTLTrack::disable();
		ofRemoveListener(events().playheadScrubbed, this, &ofxTLWMFVideoTrack::playheadScrubbed);
	}
}

bool ofxTLWMFVideoTrack::togglePlay() 
{
	if (!isLoaded()) return false;

	if (getIsPlaying()) 
	{
		stop();
	}
	else 
	{
		play();
	}
	return getIsPlaying();
}

void ofxTLWMFVideoTrack::play() 
{
	if (isLoaded() && !getIsPlaying()) 
	{
		if (player->getIsMovieDone()) 
		{
			player->setFrame(inFrame);
		}
		if (getCurrentFrame() >= timeline->getOutFrame()) 
		{
			player->setFrame(timeline->getInFrame());
		}
//		#ifdef OF_VIDEO_PLAYER_QUICKTIME
		player->setSpeed(1.0f);
//		#endif
		player->play();
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackStarted, args);

		currentlyPlaying = true;
	}
}

void ofxTLWMFVideoTrack::stop() 
{
	if (!isLoaded()) return;

	player->setSpeed(0.0f);
	if (isLoaded() && getIsPlaying()) 
	{
//		cout << "stopping playback" << endl;
		//player->stop();
		player->setPaused(true);
		ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
		ofNotifyEvent(events().playbackEnded, args);
		currentlyPlaying = false;
//		cout << "player is playing? " << player->isPlaying() << endl;
		if (timeline->getTimecontrolTrack() == this) 
		{
			timeline->setPercentComplete(player->getPosition());
		}
	}
}

bool ofxTLWMFVideoTrack::getIsPlaying() 
{
	//return isLoaded() && player->isPlaying() && player->getSpeed() > 0.0;
	return isLoaded() && currentlyPlaying;
}


void ofxTLWMFVideoTrack::setPlayAlongToTimeline(bool playAlong) 
{
	this->playAlongToTimeline = playAlong;
}

bool ofxTLWMFVideoTrack::getPlayAlongToTimeline() 
{
	return playAlongToTimeline;
}

//void ofxTLWMFVideoTrack::update(ofEventArgs& args){
void ofxTLWMFVideoTrack::update() 
{
	if (!isLoaded()) return;

	if (timeline->getTimecontrolTrack() == this) 
	{
		if (timeline->getIsFrameBased()) 
		{
			timeline->setCurrentFrame(player->getCurrentFrame());
		}
		else 
		{
			timeline->setCurrentTimeSeconds(player->getPosition()*player->getDuration());
		}

		if (getIsPlaying()) 
		{
			if (player->getCurrentFrame() < inFrame || player->getCurrentFrame() > outFrame) 
			{
				player->setFrame(inFrame);
			}

			if (selectedFrame > player->getCurrentFrame()) 
			{
				currentLoop++;
			}

			if (timeline->getOutFrame() <= player->getCurrentFrame() || player->getIsMovieDone()) 
			{
				if (timeline->getLoopType() == OF_LOOP_NONE) 
				{
					stop();
				}
				else 
				{
					int loopFrame = timeline->getInFrame();
					selectFrame(loopFrame);
					if (playAlongToTimeline) 
					{
						player->play();
					}
					ofxTLPlaybackEventArgs args = timeline->createPlaybackEvent();
					ofNotifyEvent(events().playbackLooped, args);
				}
			}
			if (timeline->getInFrame() > player->getCurrentFrame()) 
			{
				int loopFrame = timeline->getInFrame();
				selectFrame(loopFrame);
			}
		}
		else 
		{
			if (player->isPlaying()) 
			{
				stop();
			}
		}
	}
	else 
	{
		if (timeline->getCurrentTime() > player->getDuration()) 
		{
			player->setPaused(true);
		}
	}

	player->update();
	selectedFrame = player->getCurrentFrame();
}

void ofxTLWMFVideoTrack::playheadScrubbed(ofxTLPlaybackEventArgs& args) 
{
	if (isLoaded() && !currentlyPlaying && playAlongToTimeline) 
	{
		selectFrame(args.currentFrame);
	}
}

float ofxTLWMFVideoTrack::positionForSecond(float second) 
{
	if (isLoaded()) 
	{
		return ofMap(second, 0, player->getDuration(), 0.0f, 1.0f, true);
	}
	return 0;
}

void ofxTLWMFVideoTrack::playbackStarted(ofxTLPlaybackEventArgs& args) 
{
	ofxTLTrack::playbackStarted(args);
	if (isLoaded() && this != timeline->getTimecontrolTrack() && playAlongToTimeline) 
	{
		float position = positionForSecond(timeline->getCurrentTime());
		if (position < 1.0f) 
		{
			player->setSpeed(1.0f);
			player->play();
			currentlyPlaying = true;
		}
		cout << "player setPosition " << position << endl;
		player->setPosition(position);
	}
}

void ofxTLWMFVideoTrack::playbackLooped(ofxTLPlaybackEventArgs& args) 
{
	if (isLoaded() && this != timeline->getTimecontrolTrack() && playAlongToTimeline) 
	{
		if (!player->isPlaying()) 
		{
			player->play();
		}
		player->setPosition(positionForSecond(timeline->getCurrentTime()));
	}
}

void ofxTLWMFVideoTrack::playbackEnded(ofxTLPlaybackEventArgs& args) 
{
	if (isLoaded() && this != timeline->getTimecontrolTrack() && playAlongToTimeline) 
	{
		player->setPaused(true);
	}
}

bool ofxTLWMFVideoTrack::load(string moviePath) 
{
	auto videoPlayer = std::shared_ptr<ofxWMFVideoPlayer>();
	if (videoPlayer->load(moviePath)) 
	{
		setPlayer(videoPlayer);
		return true;
	}

	ofLogError(__FUNCTION__) << "Unable to load movie at path " << moviePath;
	return false;
}

void ofxTLWMFVideoTrack::setPlayer(ofxWMFVideoPlayer& newPlayer) 
{
	setPlayer(std::shared_ptr<ofxWMFVideoPlayer>(&newPlayer));
}

void ofxTLWMFVideoTrack::setPlayer(std::shared_ptr<ofxWMFVideoPlayer> newPlayer) 
{
	player = newPlayer;
	if (player->isLoaded()) 
	{
		inFrame = 0;
		outFrame = player->getTotalNumFrames();
		currentlyPlaying = false;
		player->setLoopState(OF_LOOP_NONE);
		player->setFrame(0);
		player->update();
	}
	else 
	{
		ofLogError(__FUNCTION__) << "Setting a video player before loading movie doesn't work!";
	}
}

std::shared_ptr<ofxWMFVideoPlayer> ofxTLWMFVideoTrack::getPlayer() 
{
	return player;
}

void ofxTLWMFVideoTrack::draw() 
{
	if (player == nullptr) 
	{
		return;
	}
	//inFrame = outFrame = -1;
	inFrame = 0;
	outFrame = player->getTotalNumFrames();

	ofPushStyle();

	int selectedFrameX = screenXForTime(timeline->getTimecode().secondsForFrame(selectedFrame));

	if (drawVideoPreview && getDrawRect().height > 10.0f) 
	{
		ofRectangle previewRect = ofRectangle(0, 0, player->getWidth() - 2, player->getHeight() - 4);
		previewRect.scaleTo(bounds, OF_ASPECT_RATIO_KEEP);
		previewRect.x = selectedFrameX + 1;
		previewRect.y = bounds.y - 1;
		player->draw(previewRect.x, previewRect.y, previewRect.width, previewRect.height);
		ofPushStyle();
		ofFill();
		ofSetColor(timeline->getColors().backgroundColor, 100);
		ofDrawRectangle(selectedFrameX + 1, bounds.y + 1, previewRect.width - 2, bounds.height - 2);
		ofPopStyle();
	}

	ofSetColor(timeline->getColors().textColor);
	ofDrawLine(selectedFrameX, bounds.y, selectedFrameX, bounds.y + bounds.height);
	timeline->getFont().drawString("F# " + ofToString(selectedFrame), selectedFrameX, bounds.y + 15);
	timeline->getFont().drawString(ofxTimecode::timecodeForSeconds(player->getPosition()*player->getDuration()), selectedFrameX, bounds.y + 30);

	if (inFrame != -1) 
	{
		ofSetLineWidth(2);
		ofSetColor(timeline->getColors().highlightColor);
		int inFrameX = screenXForTime(timeline->getTimecode().secondsForFrame(inFrame));
		int outFrameX = screenXForTime(timeline->getTimecode().secondsForFrame(outFrame));
		ofDrawLine(inFrameX, bounds.y, inFrameX, bounds.y + bounds.height);
		ofDrawLine(outFrameX, bounds.y, outFrameX, bounds.y + bounds.height);
		ofSetColor(timeline->getColors().textColor);
		timeline->getFont().drawString("in  " + ofToString(inFrame), inFrameX + 5, bounds.y + 10);
		timeline->getFont().drawString("out " + ofToString(outFrame), outFrameX + 5, bounds.y + bounds.height - 20);
	}

	ofPopStyle();
}

void ofxTLWMFVideoTrack::setInFrame(int in) 
{
	inFrame = in;
}

void ofxTLWMFVideoTrack::setOutFrame(int out) 
{
	outFrame = out;
}

bool ofxTLWMFVideoTrack::mousePressed(ofMouseEventArgs& args, long millis) 
{
	ofxTLTrack::mousePressed(args, millis);
	if (isActive()) 
	{
		timeline->unselectAll();
		selectFrame(timeline->getTimecode().frameForMillis(millis));

		if (timeline->getMovePlayheadOnDrag()) 
		{
			timeline->setPercentComplete(screenXtoNormalizedX(args.x));
		}
	}
	return false;
}

void ofxTLWMFVideoTrack::mouseMoved(ofMouseEventArgs& args, long millis) 
{
	ofxTLTrack::mouseMoved(args, millis);
}

void ofxTLWMFVideoTrack::mouseDragged(ofMouseEventArgs& args, long millis) 
{
	ofxTLTrack::mouseDragged(args, millis);
	if (isActive())
	{
		selectFrame(timeline->getTimecode().frameForMillis(millis));
		if (timeline->getMovePlayheadOnDrag()) 
		{
			timeline->setPercentComplete(screenXtoNormalizedX(args.x));
		}
	}
}

bool ofxTLWMFVideoTrack::isLoaded() 
{
	return player != nullptr && player->isLoaded();
}

void ofxTLWMFVideoTrack::setDrawVideoPreview(bool drawPreview) 
{
	drawVideoPreview = drawPreview;
}

bool ofxTLWMFVideoTrack::getDrawVideoPreview() 
{
	return drawVideoPreview;
}

//width and height of image elements
float ofxTLWMFVideoTrack::getContentWidth() 
{
	return isLoaded() ? player->getWidth() : 0.0f;
}

float ofxTLWMFVideoTrack::getContentHeight() 
{
	return isLoaded() ? player->getHeight() : 0.0f;
}


void ofxTLWMFVideoTrack::keyPressed(ofKeyEventArgs& args) 
{
	if (isLoaded() && hasFocus()) 
	{
		if (args.key == OF_KEY_LEFT) 
		{
			selectFrame(MAX(selectedFrame - 1, 0));
			timeline->setCurrentFrame(player->getCurrentFrame());
		}
		else if (args.key == OF_KEY_RIGHT) 
		{
			selectFrame(MIN(selectedFrame + 1, player->getTotalNumFrames() - 1));
			timeline->setCurrentFrame(player->getCurrentFrame());
		}
	}
}

int ofxTLWMFVideoTrack::selectFrame(int frame) 
{
	if (outFrame - inFrame > 0) 
	{
		selectedFrame = inFrame + (frame % (outFrame - inFrame));
		cout << "setting frame to " << selectedFrame << " with requested frame " << frame << endl;
		currentLoop = frame / (outFrame - inFrame);
		cout << "selecting frame " << selectedFrame << endl;
		player->setFrame(selectedFrame);
		timeline->flagUserChangedValue();
		player->update();
		cout << "selectFrame: player reports frame " << player->getCurrentFrame() << " with requested frame " << frame << endl;
		cout << "selecting frame " << frame << " video frame " << selectedFrame << " current loop " << currentLoop << " duration " << player->getTotalNumFrames() << " timeline duration " << timeline->getDurationInFrames() << endl;
	}
	return selectedFrame;
}

int ofxTLWMFVideoTrack::getCurrentFrame() 
{
	return (player->getCurrentFrame() - inFrame) + currentLoop*(outFrame - inFrame);
}

float ofxTLWMFVideoTrack::getCurrentTime() 
{
	//TODO: account for in and out frame.
	return player->getPosition() * player->getDuration();
}

float ofxTLWMFVideoTrack::getDuration() 
{
	return player->getDuration();
}

int ofxTLWMFVideoTrack::getSelectedFrame() 
{
	return selectedFrame + currentLoop * (outFrame - inFrame);
}

string ofxTLWMFVideoTrack::getTrackType() 
{
	return "VideoWMF";
}

#endif  // TIMELINE_VIDEO_INCLUDED
