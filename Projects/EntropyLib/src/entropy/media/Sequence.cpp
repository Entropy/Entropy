#include "Sequence.h"

#include "entropy/Helpers.h"
#include "ofxTimeline.h"
#include "App.h"

namespace entropy
{
	namespace media
	{
		//--------------------------------------------------------------
		Sequence::Sequence()
			: Base(Type::Sequence)
			, wasLoaded(false)
		{}

		//--------------------------------------------------------------
		Sequence::~Sequence()
		{}

		//--------------------------------------------------------------
		void Sequence::init()
		{
			this->parameterListeners.push_back(this->parameters.fps.newListener([this](int & value)
			{
				this->imageSequence.setFrameRate(value);
			}));
		}

		//--------------------------------------------------------------
		void Sequence::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Sequence::exit()
		{
			this->imageSequence.unloadSequence();
		}

		//--------------------------------------------------------------
		void Sequence::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				// Add a new switch if none exist.
				this->addDefaultSwitch();
				
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}

			if (!this->isLoaded())
			{
				cout << "still loading..." << endl;
				return;
			}

			if (this->switchMillis >= 0.0f)
			{
				this->renderFrame = true;
				cout << "renderFrame " << this->getContentWidth() << "x" << this->getContentHeight() << endl;

				float durationMillis = this->imageSequence.getLengthInSeconds() * 1000.0f;
				bool shouldPlay = durationMillis > 0.0f && (this->parameters.loop || durationMillis >= this->switchMillis);

				if (this->parameters.syncToTimeline)
				{
					if (shouldPlay)
					{
						if (!this->playing)
						{
							this->playing = true;
							this->startTimeMillis = ofGetElapsedTimeMillis();
						}

						// Scrub the video.
						float positionMillis = this->switchMillis;
						while (positionMillis > durationMillis)
						{
							positionMillis -= durationMillis;
						}
						this->imageSequence.setFrameAtPercent(positionMillis / durationMillis);
					}
				}
				else
				{
					if (shouldPlay)
					{
						if (!this->playing)
						{
							// Set the starting position.
							float positionMillis = this->switchMillis;
							while (positionMillis > durationMillis)
							{
								positionMillis -= durationMillis;
							}

							this->playing = true;
							this->startTimeMillis = ofGetElapsedTimeMillis();
							this->startPosMillis = positionMillis;

							this->imageSequence.setFrameAtPercent(positionMillis / durationMillis);
						}
						else
						{
							float elapsedTimeMillis = ofGetElapsedTimeMillis() - this->startTimeMillis;
							float positionMillis = this->startPosMillis + elapsedTimeMillis;
							while (positionMillis > durationMillis)
							{
								positionMillis -= durationMillis;
							}
							this->imageSequence.setFrameAtPercent(positionMillis / durationMillis);
						}
					}
					else if (!shouldPlay)
					{
						this->playing = false;
						this->renderFrame = false;
					}
				}
			}
			else if (this->playing)
			{
				this->playing = false;
				this->renderFrame = false;
			}
		}

		//--------------------------------------------------------------
		void Sequence::gui(ofxImGui::Settings & settings)
		{
			if (ofxImGui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a sequence folder.", true, GetSharedAssetsPath().string());
					if (result.bSuccess)
					{
						if (this->loadSequence(result.filePath))
						{
							const auto relativePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
							const auto testPath = GetSharedAssetsPath().append(relativePath);
							if (ofFile::doesFileExist(testPath.string()))
							{
								this->parameters.folderPath = relativePath;
							}
							else
							{
								this->parameters.folderPath = result.filePath;
							}
						}
					}
				}
				ImGui::Text("Folder: %s", this->folderName.c_str());

				ofxImGui::EndTree(settings);
			}

			ofxImGui::AddParameter(this->parameters.loop);
			ofxImGui::AddParameter(this->parameters.syncToTimeline);
		}

		//--------------------------------------------------------------
		void Sequence::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.folderPath->empty())
			{
				const auto folderPath = this->parameters.folderPath.get();
				if (ofFilePath::isAbsolute(folderPath))
				{
					this->loadSequence(folderPath);
				}
				else
				{
					this->loadSequence(GetSharedAssetsPath().string() + folderPath);
				}
			}
		}

		//--------------------------------------------------------------
		bool Sequence::loadSequence(const std::filesystem::path & folderPath)
		{
			if (!ofFile::doesFileExist(folderPath))
			{
				ofLogError(__FUNCTION__) << "No folder found at " << folderPath;
				return false;
			}

			this->imageSequence.unloadSequence();
			this->imageSequence.enableThreadedLoad(true);
			this->imageSequence.setExtension("jpg");
			this->imageSequence.loadSequence(folderPath.string());
			
			this->folderName = ofFilePath::getFileName(folderPath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Sequence::isLoaded() const
		{
			return (this->imageSequence.isLoaded() && this->getContentWidth() > 0.0f && this->getContentHeight() > 0.0f);
		}

		//--------------------------------------------------------------
		float Sequence::getContentWidth() const
		{
			return this->imageSequence.getWidth();
		}

		//--------------------------------------------------------------
		float Sequence::getContentHeight() const
		{
			return this->imageSequence.getHeight();
		}

		//--------------------------------------------------------------
		void Sequence::renderContent()
		{
			if (this->isLoaded() && this->renderFrame)
			{
				this->imageSequence.getTexture().drawSubsection(this->dstBounds, this->srcBounds);
			}
		}

		//--------------------------------------------------------------
		unsigned long long Sequence::getContentDurationMs() const
		{
			if (this->isLoaded())
			{
				return this->imageSequence.getLengthInSeconds() * 1000;
			}
			return 0;
		}
	}
}
