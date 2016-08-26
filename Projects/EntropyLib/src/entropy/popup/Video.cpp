#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Video::Video()
			: Base(Type::Video)
			, wasLoaded(false)
		{}

		//--------------------------------------------------------------
		Video::~Video()
		{}

		//--------------------------------------------------------------
		void Video::setup()
		{
			this->wasLoaded = false;
		}

		//--------------------------------------------------------------
		void Video::exit()
		{
			this->video.close();
		}

		//--------------------------------------------------------------
		void Video::update(double dt)
		{
			if (!wasLoaded && this->isLoaded())
			{
				// Adjust the bounds once the video is loaded.
				this->boundsDirty = true;
				wasLoaded = true;
			}
			
			this->video.update();
		}

		//--------------------------------------------------------------
		void Video::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ImGui::CollapsingHeader("File", nullptr, true, true))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a video file.", false, GetSharedAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadVideo(result.filePath))
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
			}
		}

		//--------------------------------------------------------------
		void Video::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadVideo(filePath);
				}
				else
				{
					this->loadVideo(GetSharedAssetsPath() + filePath);
				}
			}
		}

		//--------------------------------------------------------------
		bool Video::loadVideo(const string & filePath)
		{
			if (!ofFile::doesFileExist(filePath))
			{
				ofLogError(__FUNCTION__) << "No video found at " << filePath;
				return false;
			}
			
			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->video.loadAsync(filePath);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			this->video.play();
			this->video.setLoopState(OF_LOOP_NORMAL);
			// TODO: Time video to ofxTimeline track

			this->fileName = ofFilePath::getFileName(filePath);
			this->wasLoaded = false;

			return true;
		}

		//--------------------------------------------------------------
		bool Video::isLoaded() const
		{
			return this->video.isLoaded();
		}

		//--------------------------------------------------------------
		float Video::getContentWidth() const
		{
			return this->video.getWidth();
		}

		//--------------------------------------------------------------
		float Video::getContentHeight() const
		{
			return this->video.getHeight();
		}

		//--------------------------------------------------------------
		void Video::renderContent()
		{
//#ifdef TARGET_WIN32
//			if (video.lockSharedTexture())
//			{
//#endif
				this->video.getTexture().drawSubsection(this->dstBounds, this->srcBounds);
//#ifdef TARGET_WIN32
//				video.unlockSharedTexture();
//			}
//#endif
		}
	}
}
