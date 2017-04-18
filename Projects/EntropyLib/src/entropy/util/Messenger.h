#pragma once

#include "ofxOsc.h"
#include "ofxPreset.h"

namespace entropy
{
	namespace util
	{
		class Messenger
		{
		public:
			Messenger();
			~Messenger();

			void update();

			void drawGui(ofxImGui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

			void sendMessage(const string & rawMessage);

		protected:
			void connectSender();
			void disconnectSender();

			void connectReceiver();
			void disconnectReceiver();

			bool parseInt(const string & candidate, int & value);
			bool parseFloat(const string & candidate, float & value);

			struct : ofParameterGroup
			{
				struct : ofParameterGroup
				{
					ofParameter<bool> autoconnect{ "Auto-Connect", true };
					ofParameter<std::string> host{ "Host", "localhost" };
					ofParameter<int> port{ "Port", 3030 };

					PARAM_DECLARE("Send", autoconnect, host, port);
				} send;

				struct : ofParameterGroup
				{
					ofParameter<bool> autoconnect{ "Auto-Connect", true };
					ofParameter<int> port{ "Port", 3031 };

					PARAM_DECLARE("Receive", autoconnect, port);
				} receive;

				PARAM_DECLARE("Messenger", send, receive);
			} parameters;

			std::vector<ofEventListener> parameterListeners;

			ofxOscSender oscSender;
			bool senderConnected;

			ofxOscReceiver oscReceiver;
			bool receiverConnected;
		};
	}
}