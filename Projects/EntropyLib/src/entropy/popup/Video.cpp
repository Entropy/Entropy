#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Video::Video()
			: Base(Type::Video)
		{}

		//--------------------------------------------------------------
		Video::~Video()
		{}

		//--------------------------------------------------------------
		void Video::exit()
		{
			this->video.close();
		}

		//--------------------------------------------------------------
		void Video::update(double dt)
		{
			this->video.update();
		}

		//--------------------------------------------------------------
		void Video::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ImGui::CollapsingHeader("File", nullptr, true, true))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select a video file.", false, GetCurrentSceneAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadVideo(result.filePath))
						{
							this->parameters.filePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
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
				this->loadVideo(GetSharedAssetsPath() + this->parameters.filePath.get());
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
				this->video.load(filePath);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			this->video.play();
			// TODO: Time video to ofxTimeline track

			this->fileName = ofFilePath::getFileName(filePath);
			this->boundsDirty = true;
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
			if (this->video.lockSharedTexture())
			{
				this->video.getTexture().drawSubsection(this->dstBounds.x, this->dstBounds.y, this->dstBounds.width, this->dstBounds.height,
														this->srcBounds.x, this->srcBounds.y, this->srcBounds.width, this->srcBounds.height);
				this->video.unlockSharedTexture();
			}
		}
	}
}
