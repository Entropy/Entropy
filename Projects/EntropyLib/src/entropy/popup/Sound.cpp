#include "Sound.h"

#include "ofGstVideoPlayer.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Sound::Sound()
			: Base(Type::Sound)
			, wasLoaded(false)
		{
			// Use a video player because it has better playback functionality.
			this->soundPlayer.setPlayer(std::shared_ptr<ofGstVideoPlayer>(new ofGstVideoPlayer()));
		}

		//--------------------------------------------------------------
		Sound::~Sound()
		{}

		//--------------------------------------------------------------
		void Sound::init()
		{
			this->parameterListeners.push_back(this->parameters.loop.newListener([this](bool & enabled)
			{
				this->soundPlayer.setLoopState(enabled ? OF_LOOP_NORMAL : OF_LOOP_NONE);
			}));
		}

		//--------------------------------------------------------------
		void Sound::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Sound::exit()
		{
			this->soundPlayer.close();
		}

		//--------------------------------------------------------------
		void Sound::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				wasLoaded = true;
			}
			
			this->soundPlayer.update();

			if (!this->isLoaded())
			{
				return;
			}

			if (this->switchMillis >= 0.0f)
			{
				float durationMillis = this->soundPlayer.getDuration() * 1000.0f;
				bool shouldPlay = durationMillis > 0.0 && (this->parameters.loop || durationMillis >= this->switchMillis);

				if (this->timeline->getIsPlaying())
				{
					if (shouldPlay && this->soundPlayer.isPaused())
					{
						// Set the starting position.
						float positionMillis = this->switchMillis;
						while (positionMillis > durationMillis)
						{
							positionMillis -= durationMillis;
						}

						this->soundPlayer.setPosition(positionMillis / durationMillis);
						this->soundPlayer.setPaused(false);
					}
					else if (!shouldPlay && !this->soundPlayer.isPaused())
					{
						this->soundPlayer.setPaused(true);
					}
				}
				else if (shouldPlay)
				{
					// Scrub the video.
					float positionMillis = this->switchMillis;
					while (positionMillis > durationMillis)
					{
						positionMillis -= durationMillis;
					}

					this->soundPlayer.setPosition(positionMillis / durationMillis);
					this->soundPlayer.setPaused(false);
				}
			}
			else if (this->switchMillis < 0.0f && !this->soundPlayer.isPaused())
			{
				this->soundPlayer.setPaused(true);
			}
		}

		//--------------------------------------------------------------
		void Sound::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ofxPreset::Gui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a sound file.", false, GetSharedAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadSound(result.filePath))
						{
							auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
							auto testPath = GetSharedAssetsPath().append(relativePath);
							if (ofFile::doesFileExist(testPath))
							{
								this->parameters.filePath = relativePath;
							}
							else
							{
								this->parameters.filePath = result.filePath;
							}
						}
					}
				}
				ImGui::Text("Filename: %s", this->fileName.c_str());

				ofxPreset::Gui::EndTree(settings);
			}

			ofxPreset::Gui::AddParameter(this->parameters.loop);
		}

		//--------------------------------------------------------------
		void Sound::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadSound(filePath);
				}
				else
				{
					this->loadSound(GetSharedAssetsPath() + filePath);
				}
			}
		}

		//--------------------------------------------------------------
		bool Sound::loadSound(const string & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No audio found at " << filePath;
				return false;
			}

			this->soundPlayer.load(filePath);
			this->soundPlayer.setLoopState(this->parameters.loop ? OF_LOOP_NORMAL : OF_LOOP_NONE);

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Sound::isLoaded() const
		{
			return (this->soundPlayer.isLoaded());
		}

		//--------------------------------------------------------------
		float Sound::getContentWidth() const
		{
			return 0.0f;
		}

		//--------------------------------------------------------------
		float Sound::getContentHeight() const
		{
			return 0.0f;
		}

		//--------------------------------------------------------------
		void Sound::renderContent()
		{}
	}
}
