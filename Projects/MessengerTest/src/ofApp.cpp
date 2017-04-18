#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
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
		ofxOscMessage m;
		this->oscReceiver.getNextMessage(m);

		ostringstream oss;
		oss << m.getAddress() << " ";
		for (int i = 0; i < m.getNumArgs(); ++i)
		{
			oss << m.getArgTypeName(i) << ":";
			if (m.getArgType(i) == OFXOSC_TYPE_INT32)
			{
				oss << m.getArgAsInt32(i);
			}
			else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT)
			{
				oss << m.getArgAsFloat(i);
			}
			else if (m.getArgType(i) == OFXOSC_TYPE_STRING)
			{
				oss << m.getArgAsString(i);
			}
			else
			{
				oss << "unknown";
			}
			oss << " ";
		}

		// Add it to the list.
		Entry entry;
		entry.message = oss.str();
		entry.timestamp = ofGetTimestampString("%Y-%M-%d %H:%M:%S");

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
		ofDrawBitmapStringHighlight(entry.timestamp + " >> " + entry.message, kLeftMargin, currY);
		currY += 15.0f;
	}
}
