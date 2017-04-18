#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define RECV_PORT 3030
#define MAX_MESSAGES 20

class ofApp : public ofBaseApp 
{
public:
	void setup() override;
	void update() override;
	void draw() override;

	ofxOscReceiver oscReceiver;

	struct Entry
	{
		std::string timestamp;
		std::string message;
	};
	
	std::list<Entry> entries;
};
