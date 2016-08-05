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
            typedef enum
            {
                CONTENT_MODE_CENTER,
                CONTENT_MODE_TOP_LEFT,
                CONTENT_MODE_SCALE_TO_FILL,
                CONTENT_MODE_SCALE_ASPECT_FILL,
                CONTENT_MODE_SCALE_ASPECT_FIT
            } ContentMode;

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
				ofParameter<int> contentMode{ "Content Mode", (int)CONTENT_MODE_CENTER, (int)CONTENT_MODE_CENTER, (int)CONTENT_MODE_SCALE_ASPECT_FIT };
		
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
