#pragma once

#include "entropy/render/Layout.h"
#include "entropy/scene/Base.h"

#ifdef TARGET_WIN32
//#define USE_WMFVIDEOPLAYER 1
#endif

#ifdef USE_WMFVIDEOPLAYER
#include "ofxWMFVideoPlayer.h"
#include "entropy/video/ofxTLWMFVideoTrack.h"
#else
#include "ofxTLVideoTrack.h"
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

			string fileName;

#ifdef USE_WMFVIDEOPLAYER
			ofxWMFVideoPlayer videoPlayer;
			ofxTLWMFVideoTrack * videoTrack;
#else
			ofVideoPlayer videoPlayer;
			ofxTLVideoTrack * videoTrack;
#endif

			void updateBounds();
			ofRectangle drawBounds;
			bool dirtyBounds;

			render::Layout layout;

			virtual ofParameterGroup & getParameters() override
			{
				return this->parameters;
			}

			struct : ofParameterGroup
			{
				ofParameter<string> videoPath{ "Video Path", "" };
				ofParameter<int> layout{ "Layout", static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Back), static_cast<int>(render::Layout::Front) };
				ofParameter<int> contentMode{ "Content Mode", (int)ContentMode::Center, (int)ContentMode::Center, (int)ContentMode::ScaleAspectFit };

				struct : ofParameterGroup
				{
					ofParameter<bool> play{ "Play", true };
					ofParameter<bool> pause{ "Pause", false };
					ofParameter<bool> loop{ "Loop", false };

					PARAM_DECLARE("Playback", play, pause, loop);
				} playback;

				PARAM_DECLARE("Video", videoPath, contentMode, playback);
			} parameters;
		};
	}
}
