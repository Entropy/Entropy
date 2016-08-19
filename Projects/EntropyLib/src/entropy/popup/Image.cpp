#include "Image.h"

#include "entropy/Helpers.h"

namespace entropy
{
	namespace popup
	{
		//--------------------------------------------------------------
		Image::Image()
			: Base(Type::Image)
		{}

		//--------------------------------------------------------------
		Image::~Image()
		{}

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
							this->parameters.filePath = ofFilePath::makeRelative(GetSharedAssetsPath(), result.filePath);
						}
					}
				}
				ImGui::Text("Filename: %s", this->fileName.c_str());
			}
		}

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
		bool Image::isLoaded() const
		{
			return this->image.isAllocated();
		}

		//--------------------------------------------------------------
		float Image::getContentWidth() const
		{
			return this->image.getWidth();
		}
		
		//--------------------------------------------------------------
		float Image::getContentHeight() const
		{
			return this->image.getHeight();
		}
		
		//--------------------------------------------------------------
		void Image::renderContent()
		{
			this->image.drawSubsection(this->dstBounds.x, this->dstBounds.y, this->dstBounds.width, this->dstBounds.height,
									   this->srcBounds.x, this->srcBounds.y, this->srcBounds.width, this->srcBounds.height);
		}
	}
}
