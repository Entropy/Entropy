#pragma once

#include "ofxWMFVideoPlayer.h"

#include "Base.h"

namespace entropy
{
	namespace scene
	{
		class Video
			: public Base
		{
		public:
			virtual string getName() const override
			{
				return "entropy::scene::Video";
			}

			Video();
			~Video();

			void setup();
			void exit();
			void resize(ofResizeEventArgs & args);

			void update(double & dt);

			void drawBack();

			void gui(ofxPreset::GuiSettings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			bool loadVideo(const string & filePath);

			ofxWMFVideoPlayer videoPlayer;
			string fileName;

			ofRectangle videoBounds;
			bool dirtyBounds;

			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> videoPath;
				ofxPreset::Parameter<bool> play{ "Play", true, false };
				ofxPreset::Parameter<bool> loop{ "Loop", false, false };
				ofxPreset::Parameter<bool> centered{ "Centered", true, false };
		
				PARAM_DECLARE("Video", videoPath, loop, centered);
			} parameters;
		};
	}
}
