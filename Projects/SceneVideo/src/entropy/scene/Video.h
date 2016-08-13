#pragma once

#include "ofxWMFVideoPlayer.h"

#include "entropy/scene/Base.h"

namespace entropy
{
	namespace scene
	{
		class Video
			: public Base
		{
		public:
            enum class ContentMode
            {
                Center,
                TopLeft,
                ScaleToFill,
                ScaleAspectFill,
                ScaleAspectFit
            };

			string getName() const override
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

			void gui(ofxPreset::Gui::Settings & settings);

			void serialize(nlohmann::json & json);
			void deserialize(const nlohmann::json & json);

		protected:
			bool loadVideo(const string & filePath);

			ofxWMFVideoPlayer videoPlayer;
			string fileName;

			ofRectangle drawBounds;
			bool dirtyBounds;

			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> videoPath{"Video Path", ""};
				ofParameter<int> contentMode{ "Content Mode", (int)ContentMode::Center, (int)ContentMode::Center, (int)ContentMode::ScaleAspectFit };
		
                struct : ofParameterGroup
                {
                    ofParameter<bool> play{ "Play", true };
                    ofParameter<bool> loop{ "Loop", false };

                    PARAM_DECLARE("Playback", play, loop);
                } playback;

				PARAM_DECLARE("Video", videoPath, contentMode, playback);
			} parameters;
		};
	}
}
