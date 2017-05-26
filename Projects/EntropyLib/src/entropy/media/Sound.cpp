#include "Sound.h"

#include "ofGstVideoPlayer.h"

#include "entropy/Helpers.h"
#include "ofxTimeline.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Sound::Sound()
			: Base(Type::Sound)
			, wasLoaded(false)
			, freePlayInit(false)
		{}

		//--------------------------------------------------------------
		Sound::~Sound()
		{}

		//--------------------------------------------------------------
		void Sound::init()
		{
			this->parameterListeners.push_back(this->parameters.loop.newListener([this](bool & enabled)
			{
				this->soundPlayer.setLoop(enabled);
			}));
			this->parameterListeners.push_back(this->parameters.syncToTimeline.newListener([this](bool & enabled)
			{
				if (!enabled)
				{
					this->freePlayInit = true;
				}
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
			this->soundPlayer.unload();
		}

		//--------------------------------------------------------------
		void Sound::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				// Add a new switch if none exist.
				this->addDefaultSwitch(); 
				wasLoaded = true;
			}
			
			if (!this->isLoaded())
			{
				return;
			}

			float durationMillis = this->soundPlayer.getDurationMS();
			bool shouldPlay = (this->switchMillis >= 0.0f) && (durationMillis > 0.0f) &&
				(this->parameters.loop || durationMillis >= this->switchMillis) && 
				(this->timeline->getIsPlaying() || !this->parameters.syncToTimeline);
			if (shouldPlay && !this->soundPlayer.isPlaying())
			{
				// Set the starting position.
				float positionMillis = this->switchMillis;
				while (positionMillis > durationMillis)
				{
					positionMillis -= durationMillis;
				}

				this->soundPlayer.play();
				this->soundPlayer.setPositionMS(positionMillis);
			}
			else if (!shouldPlay && this->soundPlayer.isPlaying())
			{
				this->soundPlayer.stop();
			}
		}

		//--------------------------------------------------------------
		void Sound::gui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a sound file.", false, GetSharedAssetsPath().string());
					if (result.bSuccess)
					{
						if (this->loadSound(result.filePath))
						{
							const auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
							const auto testPath = GetSharedAssetsPath().append(relativePath);
							if (ofFile::doesFileExist(testPath.string()))
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

				ofxImGui::EndTree(settings);
			}

			ofxImGui::AddParameter(this->parameters.loop);
			ofxImGui::AddParameter(this->parameters.syncToTimeline);
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
					this->loadSound(GetSharedAssetsPath().string() + filePath);
				}
			}
		}

		//--------------------------------------------------------------
		bool Sound::loadSound(const std::filesystem::path & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No audio found at " << filePath;
				return false;
			}

			this->soundPlayer.load(filePath.string());
			this->soundPlayer.setLoop(this->parameters.loop);

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

		//--------------------------------------------------------------
		unsigned long long Sound::getContentDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->soundPlayer.getDurationMS();
			}
			return 0;
		}
	}
}
