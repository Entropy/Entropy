#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
	if (this->oscSender.setup(SEND_HOST, SEND_PORT))
	{
		ofLogNotice(__FUNCTION__) << "Sending OSC messages to "<< SEND_HOST << " on port " << SEND_PORT;
	}
	else
	{
		ofLogError(__FUNCTION__) << "Error setting up OSC sender to host " << SEND_HOST << " on port " << SEND_PORT;
	}
	
	if (this->oscReceiver.setup(RECV_PORT))
	{
		ofLogNotice(__FUNCTION__) << "Listening for OSC messages on port " << RECV_PORT;
	}
	else
	{
		ofLogError(__FUNCTION__) << "Error setting up OSC receiver on port " << RECV_PORT;
	}
}

//--------------------------------------------------------------
void ofApp::update()
{
	// Check for waiting messages.
	while (this->oscReceiver.hasWaitingMessages())
	{
		// Get the next message.
		ofxOscMessage message;
		this->oscReceiver.getNextMessage(message);

		Entry entry;
		this->buildEntry(message, entry);
		entry.type = "RECV";
		this->entries.push_front(entry);
	}

	// Delete any old messages.
	while (this->entries.size() > MAX_MESSAGES)
	{
		this->entries.pop_back();
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	ofBackgroundGradient(100, 0);

	static const float kLeftMargin = 10.0f;
	float currY = 20.0f;

	// Draw header.
	ofDrawBitmapStringHighlight("Listening for OSC messages on port " + ofToString(RECV_PORT), kLeftMargin, currY);

	// Draw recent messages.
	currY += 20.0f;
	for (auto & entry : this->entries)
	{
		ofDrawBitmapStringHighlight(entry.type + " >> " + entry.timestamp + " >> " + entry.message, kLeftMargin, currY);
		currY += 15.0f;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
	ofxOscMessage message;
	message.setAddress("/testies");
	message.addFloatArg(x);
	message.addFloatArg(y);
	message.addIntArg(button);

	Entry entry;
	this->buildEntry(message, entry);
	entry.type = "SEND";
	this->entries.push_front(entry);

	this->oscSender.sendMessage(message);
}

//--------------------------------------------------------------
void ofApp::buildEntry(const ofxOscMessage & message, Entry & entry)
{
	ostringstream oss;
	oss << message.getAddress() << " ";
	for (int i = 0; i < message.getNumArgs(); ++i)
	{
		oss << message.getArgTypeName(i) << ":";
		if (message.getArgType(i) == OFXOSC_TYPE_INT32)
		{
			oss << message.getArgAsInt32(i);
		}
		else if (message.getArgType(i) == OFXOSC_TYPE_FLOAT)
		{
			oss << message.getArgAsFloat(i);
		}
		else if (message.getArgType(i) == OFXOSC_TYPE_STRING)
		{
			oss << message.getArgAsString(i);
		}
		else
		{
			oss << "unknown";
		}
		oss << " ";
	}

	entry.message = oss.str();
	entry.timestamp = ofGetTimestampString("%Y-%M-%d %H:%M:%S");
}

