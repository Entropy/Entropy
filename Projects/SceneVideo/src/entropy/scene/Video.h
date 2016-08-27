#pragma once

#include "entropy/render/Layout.h"
#include "entropy/scene/Base.h"

#ifdef TARGET_WIN32
#include "ofxWMFVideoPlayer.h"
#endif

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

			void clear() override;

			void setup() override;
			void exit() override;

			void resizeBack(ofResizeEventArgs & args) override;
			void resizeFront(ofResizeEventArgs & args) override;

			void update(double dt) override;

			void drawBackBase() override;
			void drawFrontBase() override;

			void gui(ofxPreset::Gui::Settings & settings) override;

			void deserialize(const nlohmann::json & json) override;

		protected:
			bool loadVideo(const string & filePath);
			void drawVideo();

#ifdef TARGET_WIN32
			ofxWMFVideoPlayer videoPlayer;
#else
			ofVideoPlayer videoPlayer;
#endif
			string fileName;

			void updateBounds();
			ofRectangle drawBounds;
			bool dirtyBounds;

			render::Layout layout;

			virtual BaseParameters & getParameters() override
			{
				return this->parameters;
			}

			struct : BaseParameters
			{
				ofParameter<string> videoPath{ "Video Path", "" };
				ofParameter<int> layout{ "Layout", static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Front) };
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
