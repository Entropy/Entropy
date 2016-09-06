#include "Sound.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Sound::Sound()
			: Base(Type::Sound)
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
		}

		//--------------------------------------------------------------
		void Sound::setup()
		{}

		//--------------------------------------------------------------
		void Sound::exit()
		{
			this->soundPlayer.unload();
		}

		//--------------------------------------------------------------
		void Sound::update(double dt)
		{
			if (this->switchMillis >= 0.0f)
			{
				bool shouldPlay = this->timeline->getIsPlaying();

				if (shouldPlay && !this->soundPlayer.isPlaying())
				{
					this->soundPlayer.play();
				}
				else if (!shouldPlay && this->soundPlayer.isPlaying())
				{
					this->soundPlayer.stop();
				}
			}
			else if (this->switchMillis < 0.0f && this->soundPlayer.isPlaying())
			{
				this->soundPlayer.stop();
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
			this->soundPlayer.setLoop(this->parameters.loop);

			this->fileName = ofFilePath::getFileName(filePath);

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
