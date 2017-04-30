#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define SEND_HOST "localhost"
#define SEND_PORT 3031
#define RECV_PORT 3030
#define MAX_MESSAGES 20

class ofApp : public ofBaseApp 
{
public:
	struct Entry
	{
		std::string type;
		std::string timestamp;
		std::string message;
	};

	void setup() override;
	void update() override;
	void draw() override;

	void mouseReleased(int x, int y, int button) override;

	void buildEntry(const ofxOscMessage & message, Entry & entry);

	ofxOscSender oscSender;
	ofxOscReceiver oscReceiver;
	
	std::list<Entry> entries;
};
