#include "Image.h"

#include "ofImage.h"
#include "entropy/Helpers.h"

namespace entropy
{
	namespace media
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
		bool Image::loadMedia(const std::filesystem::path & filePath)
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

		//--------------------------------------------------------------
		bool Image::initFreePlay()
		{
			return false;
		}

		//--------------------------------------------------------------
		uint64_t Image::getCurrentTimeMs() const
		{
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Image::getCurrentFrame() const
		{
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Image::getPlaybackTimeMs()
		{
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Image::getPlaybackFrame()
		{
			return 0;
		}

		//--------------------------------------------------------------
		uint64_t Image::getDurationMs() const
		{
			return 5 * 1000;
		}

		//--------------------------------------------------------------
		uint64_t Image::getDurationFrames() const
		{
			return 1;
		}
	}
}
