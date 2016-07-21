#include "Image.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Image::Image()
			: Base(TYPE_IMAGE)
		{
			ENTROPY_POPUP_SETUP_LISTENER;
		}

		//--------------------------------------------------------------
		Image::~Image()
		{}

		//--------------------------------------------------------------
		void Image::setup()
		{
			ENTROPY_POPUP_EXIT_LISTENER;
			ENTROPY_POPUP_GUI_LISTENER;
			ENTROPY_POPUP_SERIALIZATION_LISTENERS;
		}

		//--------------------------------------------------------------
		void Image::exit()
		{
			this->image.clear();
		}

		//--------------------------------------------------------------
		void Image::gui(ofxPreset::Gui::Settings & settings)
		{
			if (ImGui::CollapsingHeader("File", nullptr, true, true))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select an image file.", false, GetCurrentSceneAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadImage(result.filePath))
						{
							this->parameters.filePath = ofFilePath::makeRelative(GetSharedAssetsPath(true), result.filePath);
						}
					}
				}
				ImGui::Text("Filename: %s", this->fileName.c_str());
			}
		}

		//--------------------------------------------------------------
		void Image::serialize(nlohmann::json & json)
		{}

		//--------------------------------------------------------------
		void Image::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				this->loadImage(GetSharedAssetsPath() + this->parameters.filePath.get());
			}
		}

		//--------------------------------------------------------------
		bool Image::loadImage(const string & filePath)
		{
			ofPixels pixels;
			ofLoadImage(pixels, filePath);
			if (!pixels.isAllocated())
			{
				ofLogError(__FUNCTION__) << "Could not load file at path " << filePath;
				return false;
			}

			bool wasUsingArbTex = ofGetUsingArbTex();
			ofDisableArbTex();
			{
				this->image.enableMipmap();
				this->image.loadData(pixels);
			}
			if (wasUsingArbTex) ofEnableArbTex();

			if (this->image.isAllocated())
			{
				this->fileName = ofFilePath::getFileName(filePath);
				this->boundsDirty = true;
				return true;
			}

			return false;
		}

		//--------------------------------------------------------------
		ofTexture & Image::getTexture()
		{
			return this->image;
		}
	}
}
