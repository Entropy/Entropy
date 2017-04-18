#include "Messenger.h"

namespace entropy
{
	namespace util
	{
		//--------------------------------------------------------------
		Messenger::Messenger()
			: senderConnected(false)
			, receiverConnected(false)
		{
			this->parameterListeners.push_back(parameters.send.autoconnect.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->connectSender();
				}
			}));
			this->parameterListeners.push_back(parameters.send.host.newListener([this](string &)
			{
				if (this->parameters.send.autoconnect)
				{
					this->connectSender();
				}
			}));
			this->parameterListeners.push_back(parameters.send.port.newListener([this](int &)
			{
				if (this->parameters.send.autoconnect)
				{
					this->connectSender();
				}
			}));
			this->parameterListeners.push_back(parameters.receive.autoconnect.newListener([this](bool & enabled)
			{
				if (enabled)
				{
					this->connectReceiver();
				}
			}));
			this->parameterListeners.push_back(parameters.receive.port.newListener([this](int &)
			{
				if (this->parameters.receive.autoconnect)
				{
					this->connectReceiver();
				}
			}));
		}

		//--------------------------------------------------------------
		Messenger::~Messenger()
		{
			oscReceiver.stop();

			this->parameterListeners.clear();
		}

		//--------------------------------------------------------------
		void Messenger::update()
		{
			while (oscReceiver.hasWaitingMessages())
			{
				// Get the next message.
				ofxOscMessage m;
				oscReceiver.getNextMessage(m);

				/*
				// check for mouse moved message
				if (m.getAddress() == "/mouse/position") {
					// both the arguments are float's
					mouseXf = m.getArgAsFloat(0);
					mouseYf = m.getArgAsFloat(1);
				}
				// check for mouse button message
				else if (m.getAddress() == "/mouse/button") {
					// first argument is int32, second is a string
					mouseButtonInt = m.getArgAsInt32(0);
					mouseButtonState = m.getArgAsString(1);
				}
				// check for an image being sent (note: the size of the image depends greatly on your network buffer sizes - if an image is too big the message won't come through )
				else if (m.getAddress() == "/image") {
					ofBuffer buffer = m.getArgAsBlob(0);
					receivedImage.load(buffer);
				}
				else {
					// unrecognized message: display on the bottom of the screen
					string msg_string;
					msg_string = m.getAddress();
					msg_string += ":";
					for (int i = 0; i < m.getNumArgs(); i++) {
						// get the argument type
						msg_string += " ";
						msg_string += m.getArgTypeName(i);
						msg_string += ":";
						// display the argument - make sure we get the right type
						if (m.getArgType(i) == OFXOSC_TYPE_INT32) {
							msg_string += ofToString(m.getArgAsInt32(i));
						}
						else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT) {
							msg_string += ofToString(m.getArgAsFloat(i));
						}
						else if (m.getArgType(i) == OFXOSC_TYPE_STRING) {
							msg_string += m.getArgAsString(i);
						}
						else {
							msg_string += "unknown";
						}
					}
					// add to the list of strings to display
					msg_strings[current_msg_string] = msg_string;
					timers[current_msg_string] = ofGetElapsedTimef() + 5.0f;
					current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
					// clear the next line
					msg_strings[current_msg_string] = "";
				}
				*/
			}
		}

		//--------------------------------------------------------------
		void Messenger::drawGui(ofxImGui::Settings & settings)
		{
			ofxImGui::SetNextWindow(settings);
			if (ofxImGui::BeginWindow(this->parameters.getName().c_str(), settings))
			{
				if (ofxImGui::BeginTree(this->parameters.send, settings))
				{
					ofxImGui::AddParameter(this->parameters.send.host);
					ofxImGui::AddStepper(this->parameters.send.port);
					ofxImGui::AddParameter(this->parameters.send.autoconnect);
					ImGui::SameLine();
					if (this->senderConnected)
					{
						if (ImGui::Button("Disconnect"))
						{
							this->disconnectSender();
						}
					}
					else
					{
						if (ImGui::Button("Connect"))
						{
							this->connectSender();
						}
					}

					ofxImGui::EndTree(settings);
				}

				if (ofxImGui::BeginTree(this->parameters.receive, settings))
				{
					ofxImGui::AddStepper(this->parameters.receive.port);
					ofxImGui::AddParameter(this->parameters.receive.autoconnect);
					ImGui::SameLine();
					if (this->receiverConnected)
					{
						if (ImGui::Button("Disconnect"))
						{
							this->disconnectReceiver();
						}
					}
					else
					{
						if (ImGui::Button("Connect"))
						{
							this->connectReceiver();
						}
					}

					ofxImGui::EndTree(settings);
				}
			}
			ofxImGui::EndWindow(settings);
		}

		//--------------------------------------------------------------
		void Messenger::serialize(nlohmann::json & json)
		{
			ofxPreset::Serializer::Serialize(json, this->parameters);
		}

		//--------------------------------------------------------------
		void Messenger::deserialize(const nlohmann::json & json)
		{
			ofxPreset::Serializer::Deserialize(json, this->parameters);
		
			if (this->parameters.send.autoconnect)
			{
				this->connectSender();
			}
			if (this->parameters.receive.autoconnect)
			{
				this->connectReceiver();
			}
		}

		//--------------------------------------------------------------
		void Messenger::connectSender()
		{
			ofxOscSenderSettings settings;
			settings.host = this->parameters.send.host;
			settings.port = this->parameters.send.port;

			this->senderConnected = this->oscSender.setup(settings);
		}

		//--------------------------------------------------------------
		void Messenger::disconnectSender()
		{
			this->oscSender.clear();
			this->senderConnected = false;
		}

		//--------------------------------------------------------------
		void Messenger::connectReceiver()
		{
			ofxOscReceiverSettings settings;
			settings.port = this->parameters.receive.port;

			this->receiverConnected = this->oscReceiver.setup(settings);
		}

		//--------------------------------------------------------------
		void Messenger::disconnectReceiver()
		{
			this->oscReceiver.stop();
			this->receiverConnected = false;
		}

		//--------------------------------------------------------------
		void Messenger::sendMessage(const string & rawMessage)
		{
			if (!this->senderConnected)
			{
				ofLogError(__FUNCTION__) << "Sender not connected!";
				return;
			}

			auto tokens = ofSplitString(rawMessage, " ", true, true);
			if (tokens[0].at(0) != '/')
			{
				ofLogError(__FUNCTION__) << "Message address must start with '/'! It is " << tokens[0];
				return;
			}

			ofxOscMessage message;
			message.setAddress(tokens.at(0));

			for (int i = 1; i < tokens.size(); ++i)
			{
				int intValue;
				if (parseInt(tokens[i], intValue))
				{
					message.addIntArg(intValue);
					continue;
				}

				float floatValue;
				if (parseFloat(tokens[i], floatValue))
				{
					message.addFloatArg(floatValue);
					continue;
				}

				message.addStringArg(tokens[i]);
			}
			
			this->oscSender.sendMessage(message);
		}

		//--------------------------------------------------------------
		bool Messenger::parseInt(const string & candidate, int & value)
		{
			int length;
			if (sscanf(candidate.c_str(), "%d%n", &value, &length) != 1 || (length != candidate.size()))
			{
				return false;
			}
			return true;
		}
		
		//--------------------------------------------------------------
		bool Messenger::parseFloat(const string & candidate, float & value)
		{
			int length;
			if (sscanf(candidate.c_str(), "%f%n", &value, &length) != 1 || (length != candidate.size()))
			{
				return false;
			}
			return true;
		}
	}
}