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
			if (ofxPreset::Gui::BeginTree("File", settings))
			{
				if (ImGui::Button("Load..."))
				{
					auto result = ofSystemLoadDialog("Select an image file.", false, GetSharedAssetsPath());
					if (result.bSuccess)
					{
						if (this->loadImage(result.filePath))
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
		}

		//--------------------------------------------------------------
		void Image::deserialize(const nlohmann::json & json)
		{
			if (!this->parameters.filePath->empty())
			{
				const auto filePath = this->parameters.filePath.get();
				if (ofFilePath::isAbsolute(filePath))
				{
					this->loadImage(filePath);
				}
				else
				{
					this->loadImage(GetSharedAssetsPath() + filePath);
				}
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
			this->image.drawSubsection(this->dstBounds, this->srcBounds);
		}
	}
}
