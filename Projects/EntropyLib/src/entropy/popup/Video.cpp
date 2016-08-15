#include "Video.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Video::Video()
			: Base(Type::Video)
		{
			ENTROPY_POPUP_SETUP_LISTENER;
		}

		//--------------------------------------------------------------
		Video::~Video()
		{}

		//--------------------------------------------------------------
		void Video::setup()
		{
			ENTROPY_POPUP_EXIT_LISTENER;
			ENTROPY_POPUP_UPDATE_LISTENER;
			ENTROPY_POPUP_GUI_LISTENER;
			ENTROPY_POPUP_SERIALIZATION_LISTENERS;
		}

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
		void Video::serialize(nlohmann::json & json)
		{}

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
			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->video.load(filePath);
				this->video.play();
			}
			if (wasUsingArbTex) ofEnableArbTex();

			this->fileName = ofFilePath::getFileName(filePath);
			this->boundsDirty = true;
			return true;
		}

		//--------------------------------------------------------------
		ofTexture & Video::getTexture()
		{
			return this->video.getTextureReference();
		}
	}
}
